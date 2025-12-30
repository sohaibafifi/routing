// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IMIPBackend.hpp"

#ifdef HIGHS_FOUND
#include <Highs.h>
#endif

#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace routing {
namespace mip {

#ifdef HIGHS_FOUND

/**
 * @brief HiGHS implementation of IMIPBackend
 *
 * Uses the open-source HiGHS solver for mixed integer programming.
 * https://highs.dev/
 */
class HiGHSMIPBackend : public IMIPBackend {
public:
    HiGHSMIPBackend()
        : solved_(false)
        , numVars_(0)
        , numConstraints_(0)
    {
        highs_.setOptionValue("output_flag", false);
        highs_.setOptionValue("log_to_console", false);
    }

    ~HiGHSMIPBackend() override = default;

    std::string name() const override { return "highs"; }

    // ========== Variable Creation ==========

    Var newVar(double lb, double ub, const std::string& name) override {
        int id = numVars_++;
        varLB_.push_back(lb);
        varUB_.push_back(ub);
        varTypes_.push_back(HighsVarType::kContinuous);
        varNames_.push_back(name);
        return Var(id);
    }

    IntVar newIntVar(int lb, int ub, const std::string& name) override {
        int id = numVars_++;
        varLB_.push_back(static_cast<double>(lb));
        varUB_.push_back(static_cast<double>(ub));
        varTypes_.push_back(HighsVarType::kInteger);
        varNames_.push_back(name);
        return IntVar(id);
    }

    BoolVar newBoolVar(const std::string& name) override {
        int id = numVars_++;
        varLB_.push_back(0.0);
        varUB_.push_back(1.0);
        varTypes_.push_back(HighsVarType::kInteger);
        varNames_.push_back(name);
        return BoolVar(id);
    }

    // ========== Constraint Addition ==========

    void addLessEqual(const LinearExpr& expr, double rhs,
                      const std::string& name) override {
        addConstraintInternal(expr, -kHighsInf, rhs, name);
    }

    void addGreaterEqual(const LinearExpr& expr, double rhs,
                         const std::string& name) override {
        addConstraintInternal(expr, rhs, kHighsInf, name);
    }

    void addEqual(const LinearExpr& expr, double rhs,
                  const std::string& name) override {
        addConstraintInternal(expr, rhs, rhs, name);
    }

    void addRange(const LinearExpr& expr, double lb, double ub,
                  const std::string& name) override {
        addConstraintInternal(expr, lb, ub, name);
    }

    void addIndicator(BoolVar indicator, int value,
                      const LinearExpr& expr, Sense sense, double rhs,
                      const std::string& name) override {
        // HiGHS doesn't have native indicator constraints
        // We implement using big-M formulation
        double M = 1e6;  // Big-M constant

        LinearExpr modified = expr;
        if (value == 1) {
            // If indicator == 1, then constraint is active
            // Convert to: expr sense rhs - M*(1 - indicator)
            switch (sense) {
                case Sense::LessEqual:
                    // expr <= rhs becomes expr - M*(1-ind) <= rhs => expr + M*ind <= rhs + M
                    modified.addTerm(indicator, M);
                    addLessEqual(modified, rhs + M, name);
                    break;
                case Sense::GreaterEqual:
                    // expr >= rhs becomes expr + M*(1-ind) >= rhs => expr - M*ind >= rhs - M
                    modified.addTerm(indicator, -M);
                    addGreaterEqual(modified, rhs - M, name);
                    break;
                case Sense::Equal:
                    // Split into two constraints
                    {
                        LinearExpr le = expr;
                        le.addTerm(indicator, M);
                        addLessEqual(le, rhs + M, name + "_le");

                        LinearExpr ge = expr;
                        ge.addTerm(indicator, -M);
                        addGreaterEqual(ge, rhs - M, name + "_ge");
                    }
                    break;
            }
        } else {
            // If indicator == 0, then constraint is active
            switch (sense) {
                case Sense::LessEqual:
                    modified.addTerm(indicator, -M);
                    addLessEqual(modified, rhs, name);
                    break;
                case Sense::GreaterEqual:
                    modified.addTerm(indicator, M);
                    addGreaterEqual(modified, rhs, name);
                    break;
                case Sense::Equal:
                    {
                        LinearExpr le = expr;
                        le.addTerm(indicator, -M);
                        addLessEqual(le, rhs, name + "_le");

                        LinearExpr ge = expr;
                        ge.addTerm(indicator, M);
                        addGreaterEqual(ge, rhs, name + "_ge");
                    }
                    break;
            }
        }
    }

    void addSOS1(const std::vector<Var>& vars,
                 const std::vector<double>& weights,
                 const std::string& name) override {
        // HiGHS doesn't support SOS constraints natively
        // We approximate using pairwise constraints (weak formulation)
        // For proper SOS1: at most one variable is non-zero
        // This is a simplification - full SOS requires more complex handling
        (void)weights;
        (void)name;

        // Add constraint: sum of binary indicators <= 1
        // This only works if vars are binary. For general case, need auxiliary binaries.
        // For now, skip - user should use binary variables directly
    }

    void addSOS2(const std::vector<Var>& vars,
                 const std::vector<double>& weights,
                 const std::string& name) override {
        // SOS2 not natively supported in HiGHS
        // Skip for now - would require auxiliary binary variables
        (void)vars;
        (void)weights;
        (void)name;
    }

    void addLazyConstraint(const LinearExpr& expr, Sense sense, double rhs) override {
        // HiGHS doesn't support lazy constraints via callbacks in the same way
        // Add as regular constraint
        switch (sense) {
            case Sense::LessEqual:
                addLessEqual(expr, rhs, "");
                break;
            case Sense::Equal:
                addEqual(expr, rhs, "");
                break;
            case Sense::GreaterEqual:
                addGreaterEqual(expr, rhs, "");
                break;
        }
    }

    void addUserCut(const LinearExpr& expr, Sense sense, double rhs) override {
        // Add as regular constraint
        addLazyConstraint(expr, sense, rhs);
    }

    // ========== Objective ==========

    void minimize(const LinearExpr& expr) override {
        setObjective(expr, ObjSense::kMinimize);
    }

    void maximize(const LinearExpr& expr) override {
        setObjective(expr, ObjSense::kMaximize);
    }

    void minimizeQuad(const QuadExpr& expr) override {
        // HiGHS supports QP - set up Hessian matrix
        setObjective(expr.linearPart(), ObjSense::kMinimize);

        if (!expr.quadTerms().empty()) {
            // Build Hessian in sparse format
            std::vector<HighsInt> qStart, qIndex;
            std::vector<double> qValue;

            // Convert quadratic terms to lower triangular Hessian
            // Note: HiGHS expects 0.5 * x' Q x, so we need to multiply by 2
            std::map<std::pair<int,int>, double> hessian;
            for (const auto& [i, j, coeff] : expr.quadTerms()) {
                int minIdx = std::min(i, j);
                int maxIdx = std::max(i, j);
                hessian[{minIdx, maxIdx}] += (minIdx == maxIdx) ? 2.0 * coeff : coeff;
            }

            // Convert to CSC format
            qStart.resize(numVars_ + 1, 0);
            for (const auto& [ij, val] : hessian) {
                qStart[ij.second + 1]++;
            }
            for (int i = 1; i <= numVars_; i++) {
                qStart[i] += qStart[i-1];
            }

            qIndex.resize(hessian.size());
            qValue.resize(hessian.size());
            std::vector<int> pos(numVars_, 0);
            for (int i = 0; i < numVars_; i++) {
                pos[i] = qStart[i];
            }
            for (const auto& [ij, val] : hessian) {
                int idx = pos[ij.second]++;
                qIndex[idx] = ij.first;
                qValue[idx] = val;
            }

            // Pass Hessian - format 1 = triangular
            highs_.passHessian(numVars_, static_cast<HighsInt>(hessian.size()),
                              1,  // kTriangular format
                              qStart.data(), qIndex.data(), qValue.data());
        }
    }

    void maximizeQuad(const QuadExpr& expr) override {
        // Negate and minimize
        QuadExpr negated;
        for (const auto& [varId, coeff] : expr.linearPart().terms()) {
            negated.addLinearTerm(Var(varId), -coeff);
        }
        for (const auto& [i, j, coeff] : expr.quadTerms()) {
            negated.addQuadTerm(Var(i), Var(j), -coeff);
        }
        minimizeQuad(negated);
        objSense_ = ObjSense::kMaximize;  // Remember to negate solution
    }

    // ========== Warm Starting ==========

    void setWarmStart(const std::vector<Var>& vars,
                      const std::vector<double>& values) override {
        HighsSolution solution;
        solution.col_value.resize(numVars_, 0.0);
        for (size_t i = 0; i < vars.size(); ++i) {
            solution.col_value[vars[i].id()] = values[i];
        }
        highs_.setSolution(solution);
    }

    void setMIPStart(const std::vector<std::pair<Var, double>>& solution) override {
        HighsSolution sol;
        sol.col_value.resize(numVars_, 0.0);
        for (const auto& [var, value] : solution) {
            sol.col_value[var.id()] = value;
        }
        highs_.setSolution(sol);
    }

    // ========== Solving ==========

    MIPStatus solve(double timeout) override {
        // Build the model
        buildModel();

        // Set time limit
        highs_.setOptionValue("time_limit", timeout);

        // Solve
        HighsStatus status = highs_.run();
        solved_ = (status == HighsStatus::kOk);

        // Map status
        HighsModelStatus modelStatus = highs_.getModelStatus();
        switch (modelStatus) {
            case HighsModelStatus::kOptimal:
                return MIPStatus::Optimal;
            case HighsModelStatus::kInfeasible:
                return MIPStatus::Infeasible;
            case HighsModelStatus::kUnbounded:
                return MIPStatus::Unbounded;
            case HighsModelStatus::kUnboundedOrInfeasible:
                return MIPStatus::InfeasibleOrUnbounded;
            case HighsModelStatus::kTimeLimit:
                if (highs_.getInfo().primal_solution_status == kSolutionStatusFeasible) {
                    return MIPStatus::TimeLimit;
                }
                return MIPStatus::TimeLimit;
            case HighsModelStatus::kIterationLimit:
            case HighsModelStatus::kSolutionLimit:
                return MIPStatus::SolutionLimit;
            default:
                if (status == HighsStatus::kError) {
                    return MIPStatus::Error;
                }
                return MIPStatus::Unknown;
        }
    }

    // ========== Solution Access ==========

    double getValue(Var var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return highs_.getSolution().col_value[var.id()];
    }

    double getObjectiveValue() const override {
        if (!solved_) return std::numeric_limits<double>::infinity();
        return highs_.getInfo().objective_function_value;
    }

    double getObjectiveBound() const override {
        if (!solved_) return 0.0;
        return highs_.getInfo().mip_dual_bound;
    }

    double getGap() const override {
        if (!solved_) return 1.0;
        return highs_.getInfo().mip_gap;
    }

    double getReducedCost(Var var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        const auto& dual = highs_.getSolution().col_dual;
        if (var.id() < static_cast<int>(dual.size())) {
            return dual[var.id()];
        }
        return 0.0;
    }

    double getDual(int constraintIndex) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        const auto& dual = highs_.getSolution().row_dual;
        if (constraintIndex < static_cast<int>(dual.size())) {
            return dual[constraintIndex];
        }
        return 0.0;
    }

    // ========== Statistics ==========

    double getSolveTime() const override {
        return highs_.getRunTime();
    }

    long long getNumIterations() const override {
        return static_cast<long long>(highs_.getInfo().simplex_iteration_count);
    }

    long long getNumNodes() const override {
        return static_cast<long long>(highs_.getInfo().mip_node_count);
    }

    int getNumSolutions() const override {
        // HiGHS doesn't track solution pool size directly
        return solved_ ? 1 : 0;
    }

    // ========== Parameters ==========

    void setTimeLimit(double seconds) override {
        highs_.setOptionValue("time_limit", seconds);
    }

    void setGapTolerance(double gap) override {
        highs_.setOptionValue("mip_rel_gap", gap);
    }

    void setNumThreads(int threads) override {
        highs_.setOptionValue("threads", threads);
    }

    void setRandomSeed(int seed) override {
        highs_.setOptionValue("random_seed", seed);
    }

    void setVerbosity(int level) override {
        highs_.setOptionValue("output_flag", level > 0);
        highs_.setOptionValue("log_to_console", level > 0);
    }

    void setPresolve(bool enable) override {
        highs_.setOptionValue("presolve", enable ? "on" : "off");
    }

    void setNodeLimit(long long limit) override {
        highs_.setOptionValue("mip_max_nodes", static_cast<HighsInt>(limit));
    }

    void setSolutionLimit(int limit) override {
        // HiGHS doesn't have a direct solution limit parameter
        (void)limit;
    }

    // ========== Model Management ==========

    void clear() override {
        highs_.clear();
        varLB_.clear();
        varUB_.clear();
        varTypes_.clear();
        varNames_.clear();
        objCoeffs_.clear();
        conLB_.clear();
        conUB_.clear();
        conStarts_.clear();
        conIndices_.clear();
        conValues_.clear();
        objOffset_ = 0.0;
        objSense_ = ObjSense::kMinimize;
        solved_ = false;
        modelBuilt_ = false;
        numVars_ = 0;
        numConstraints_ = 0;
    }

    int getNumVars() const override {
        return numVars_;
    }

    int getNumConstraints() const override {
        return numConstraints_;
    }

    long long getNumNonZeros() const override {
        return static_cast<long long>(conIndices_.size());
    }

    bool exportModel(const std::string& filename) const override {
        if (!modelBuilt_) {
            // Model not built yet - user should solve first or call a build method
            return false;
        }
        return highs_.writeModel(filename) == HighsStatus::kOk;
    }

    bool importModel(const std::string& filename) override {
        clear();
        return highs_.readModel(filename) == HighsStatus::kOk;
    }

    // ========== Advanced ==========

    MIPStatus solveRelaxation() override {
        // Temporarily change all integer/binary to continuous
        std::vector<HighsVarType> savedTypes = varTypes_;
        for (auto& t : varTypes_) {
            t = HighsVarType::kContinuous;
        }

        buildModel();
        MIPStatus status = solve(highs_.getOptions().time_limit);

        varTypes_ = savedTypes;
        return status;
    }

    MIPStatus solveFixed() override {
        if (!solved_) {
            throw std::runtime_error("Must have a solution to fix variables");
        }

        // Fix all variables to their solution values
        const auto& sol = highs_.getSolution();
        std::vector<double> savedLB = varLB_;
        std::vector<double> savedUB = varUB_;

        for (int i = 0; i < numVars_; ++i) {
            varLB_[i] = sol.col_value[i];
            varUB_[i] = sol.col_value[i];
        }

        // Make all continuous
        std::vector<HighsVarType> savedTypes = varTypes_;
        for (auto& t : varTypes_) {
            t = HighsVarType::kContinuous;
        }

        buildModel();
        MIPStatus status = solve(highs_.getOptions().time_limit);

        varLB_ = savedLB;
        varUB_ = savedUB;
        varTypes_ = savedTypes;

        return status;
    }

    // ========== HiGHS-Specific Access ==========

    Highs& getHiGHS() { return highs_; }
    const Highs& getHiGHS() const { return highs_; }

private:
    mutable Highs highs_;  // mutable for const methods like exportModel
    bool solved_;
    bool modelBuilt_ = false;
    int numVars_;
    int numConstraints_;
    ObjSense objSense_ = ObjSense::kMinimize;

    // Variable data
    std::vector<double> varLB_;
    std::vector<double> varUB_;
    std::vector<HighsVarType> varTypes_;
    std::vector<std::string> varNames_;

    // Objective
    std::vector<double> objCoeffs_;
    double objOffset_ = 0.0;

    // Constraints in CSR format
    std::vector<double> conLB_;
    std::vector<double> conUB_;
    std::vector<HighsInt> conStarts_;
    std::vector<HighsInt> conIndices_;
    std::vector<double> conValues_;

    void addConstraintInternal(const LinearExpr& expr, double lb, double ub,
                               const std::string& name) {
        (void)name;
        conLB_.push_back(lb - expr.constant());
        conUB_.push_back(ub - expr.constant());

        conStarts_.push_back(static_cast<HighsInt>(conIndices_.size()));
        for (const auto& [varId, coeff] : expr.terms()) {
            conIndices_.push_back(static_cast<HighsInt>(varId));
            conValues_.push_back(coeff);
        }

        numConstraints_++;
    }

    void setObjective(const LinearExpr& expr, ObjSense sense) {
        objCoeffs_.assign(numVars_, 0.0);
        for (const auto& [varId, coeff] : expr.terms()) {
            objCoeffs_[varId] += coeff;
        }
        objOffset_ = expr.constant();
        objSense_ = sense;
    }

    void buildModel() {
        highs_.clear();

        if (numVars_ == 0) return;

        // Prepare model
        HighsModel model;
        model.lp_.num_col_ = numVars_;
        model.lp_.num_row_ = numConstraints_;

        // Variables
        model.lp_.col_lower_ = varLB_;
        model.lp_.col_upper_ = varUB_;

        // Objective
        if (objCoeffs_.empty()) {
            objCoeffs_.resize(numVars_, 0.0);
        }
        model.lp_.col_cost_ = objCoeffs_;
        model.lp_.offset_ = objOffset_;
        model.lp_.sense_ = objSense_;

        // Constraints
        model.lp_.row_lower_ = conLB_;
        model.lp_.row_upper_ = conUB_;

        // Finalize constraint starts
        conStarts_.push_back(static_cast<HighsInt>(conIndices_.size()));

        model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;
        model.lp_.a_matrix_.start_ = conStarts_;
        model.lp_.a_matrix_.index_ = conIndices_;
        model.lp_.a_matrix_.value_ = conValues_;

        // Variable types (integer/binary)
        model.lp_.integrality_ = varTypes_;

        highs_.passModel(model);
        modelBuilt_ = true;
    }
};

#else // !HIGHS_FOUND

/**
 * @brief Stub implementation when HiGHS is not available
 */
class HiGHSMIPBackend : public IMIPBackend {
public:
    HiGHSMIPBackend() {
        throw std::runtime_error("HiGHS is not available. Install HiGHS and rebuild.");
    }

    std::string name() const override { return "highs"; }

    Var newVar(double, double, const std::string&) override { return Var(); }
    IntVar newIntVar(int, int, const std::string&) override { return IntVar(); }
    BoolVar newBoolVar(const std::string&) override { return BoolVar(); }

    void addLessEqual(const LinearExpr&, double, const std::string&) override {}
    void addGreaterEqual(const LinearExpr&, double, const std::string&) override {}
    void addEqual(const LinearExpr&, double, const std::string&) override {}
    void addRange(const LinearExpr&, double, double, const std::string&) override {}
    void addIndicator(BoolVar, int, const LinearExpr&, Sense, double, const std::string&) override {}
    void addSOS1(const std::vector<Var>&, const std::vector<double>&, const std::string&) override {}
    void addSOS2(const std::vector<Var>&, const std::vector<double>&, const std::string&) override {}
    void addLazyConstraint(const LinearExpr&, Sense, double) override {}
    void addUserCut(const LinearExpr&, Sense, double) override {}

    void minimize(const LinearExpr&) override {}
    void maximize(const LinearExpr&) override {}
    void minimizeQuad(const QuadExpr&) override {}
    void maximizeQuad(const QuadExpr&) override {}

    void setWarmStart(const std::vector<Var>&, const std::vector<double>&) override {}
    void setMIPStart(const std::vector<std::pair<Var, double>>&) override {}

    MIPStatus solve(double) override { return MIPStatus::Error; }

    double getValue(Var) const override { return 0.0; }
    double getObjectiveValue() const override { return 0.0; }
    double getObjectiveBound() const override { return 0.0; }
    double getReducedCost(Var) const override { return 0.0; }
    double getDual(int) const override { return 0.0; }

    double getSolveTime() const override { return 0.0; }
    long long getNumIterations() const override { return 0; }
    long long getNumNodes() const override { return 0; }
    int getNumSolutions() const override { return 0; }

    void setTimeLimit(double) override {}
    void setGapTolerance(double) override {}
    void setNumThreads(int) override {}
    void setRandomSeed(int) override {}
    void setVerbosity(int) override {}
    void setPresolve(bool) override {}
    void setNodeLimit(long long) override {}
    void setSolutionLimit(int) override {}

    void clear() override {}
    int getNumVars() const override { return 0; }
    int getNumConstraints() const override { return 0; }
    long long getNumNonZeros() const override { return 0; }
    bool exportModel(const std::string&) const override { return false; }
    bool importModel(const std::string&) override { return false; }

    MIPStatus solveRelaxation() override { return MIPStatus::Error; }
    MIPStatus solveFixed() override { return MIPStatus::Error; }
};

#endif // HIGHS_FOUND

} // namespace mip
} // namespace routing
