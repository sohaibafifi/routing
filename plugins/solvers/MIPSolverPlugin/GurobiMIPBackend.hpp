// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IMIPBackend.hpp"

#ifdef GUROBI_FOUND
#include <gurobi_c++.h>
#endif

#include <stdexcept>
#include <vector>
#include <memory>
#include <limits>

namespace routing {
namespace mip {

#ifdef GUROBI_FOUND

/**
 * @brief Gurobi implementation of IMIPBackend
 *
 * Uses Gurobi for mixed integer programming.
 */
class GurobiMIPBackend : public IMIPBackend {
public:
    GurobiMIPBackend()
        : env_()
        , model_(std::make_unique<GRBModel>(env_))
        , solved_(false)
        , time_limit_(3600.0)
        , constraintCount_(0)
        , model_dirty_(false)
        , output_flag_(0)
    {
        env_.set(GRB_IntParam_OutputFlag, 0);
        model_->set(GRB_IntParam_OutputFlag, 0);
    }

    ~GurobiMIPBackend() override = default;

    std::string name() const override { return "gurobi"; }

    // ========== Variable Creation ==========

    Var newVar(double lb, double ub, const std::string& name) override {
        int id = static_cast<int>(vars_.size());
        GRBVar var = model_->addVar(lb, ub, 0.0, GRB_CONTINUOUS, name);
        vars_.push_back(var);
        varTypes_.push_back(GRB_CONTINUOUS);
        markDirty();
        return Var(id);
    }

    IntVar newIntVar(int lb, int ub, const std::string& name) override {
        int id = static_cast<int>(vars_.size());
        GRBVar var = model_->addVar(static_cast<double>(lb),
                                    static_cast<double>(ub),
                                    0.0,
                                    GRB_INTEGER,
                                    name);
        vars_.push_back(var);
        varTypes_.push_back(GRB_INTEGER);
        markDirty();
        return IntVar(id);
    }

    BoolVar newBoolVar(const std::string& name) override {
        int id = static_cast<int>(vars_.size());
        GRBVar var = model_->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
        vars_.push_back(var);
        varTypes_.push_back(GRB_BINARY);
        markDirty();
        return BoolVar(id);
    }

    // ========== Constraint Addition ==========

    void addLessEqual(const LinearExpr& expr, double rhs,
                      const std::string& name) override {
        addLinearConstraint(expr, GRB_LESS_EQUAL, rhs, name);
    }

    void addGreaterEqual(const LinearExpr& expr, double rhs,
                         const std::string& name) override {
        addLinearConstraint(expr, GRB_GREATER_EQUAL, rhs, name);
    }

    void addEqual(const LinearExpr& expr, double rhs,
                  const std::string& name) override {
        addLinearConstraint(expr, GRB_EQUAL, rhs, name);
    }

    void addRange(const LinearExpr& expr, double lb, double ub,
                  const std::string& name) override {
        GRBLinExpr grb = toGRBLinExpr(expr);
        GRBConstr constr = model_->addRange(grb, lb, ub, name);
        constraints_.push_back(constr);
        constraintCount_++;
        markDirty();
    }

    void addIndicator(BoolVar indicator, int value,
                      const LinearExpr& expr, Sense sense, double rhs,
                      const std::string& name) override {
        GRBLinExpr grb = toGRBLinExpr(expr);
        GRBGenConstr gen = model_->addGenConstrIndicator(
            getVar(indicator),
            value,
            grb,
            toGRBSense(sense),
            rhs,
            name
        );
        genConstraints_.push_back(gen);
        constraintCount_++;
        markDirty();
    }

    void addSOS1(const std::vector<Var>& vars,
                 const std::vector<double>& weights,
                 const std::string& name) override {
        addSOS(vars, weights, GRB_SOS_TYPE1, name);
    }

    void addSOS2(const std::vector<Var>& vars,
                 const std::vector<double>& weights,
                 const std::string& name) override {
        addSOS(vars, weights, GRB_SOS_TYPE2, name);
    }

    void addLazyConstraint(const LinearExpr& expr, Sense sense, double rhs) override {
        GRBConstr constr = model_->addConstr(
            toGRBLinExpr(expr),
            toGRBSense(sense),
            rhs,
            ""
        );
        constr.set(GRB_IntAttr_Lazy, 1);
        constraints_.push_back(constr);
        constraintCount_++;
        markDirty();
    }

    void addUserCut(const LinearExpr& expr, Sense sense, double rhs) override {
        addLinearConstraint(expr, toGRBSense(sense), rhs, "");
    }

    // ========== Objective ==========

    void minimize(const LinearExpr& expr) override {
        model_->setObjective(toGRBLinExpr(expr), GRB_MINIMIZE);
        markDirty();
    }

    void maximize(const LinearExpr& expr) override {
        model_->setObjective(toGRBLinExpr(expr), GRB_MAXIMIZE);
        markDirty();
    }

    void minimizeQuad(const QuadExpr& expr) override {
        model_->setObjective(toGRBQuadExpr(expr), GRB_MINIMIZE);
        markDirty();
    }

    void maximizeQuad(const QuadExpr& expr) override {
        model_->setObjective(toGRBQuadExpr(expr), GRB_MAXIMIZE);
        markDirty();
    }

    // ========== Warm Starting ==========

    void setWarmStart(const std::vector<Var>& vars,
                      const std::vector<double>& values) override {
        for (size_t i = 0; i < vars.size(); ++i) {
            vars_.at(vars[i].id()).set(GRB_DoubleAttr_Start, values[i]);
        }
        markDirty();
    }

    void setMIPStart(const std::vector<std::pair<Var, double>>& solution) override {
        for (const auto& [var, value] : solution) {
            vars_.at(var.id()).set(GRB_DoubleAttr_Start, value);
        }
        markDirty();
    }

    // ========== Solving ==========

    MIPStatus solve(double timeout) override {
        try {
            model_->set(GRB_DoubleParam_TimeLimit, timeout);
            time_limit_ = timeout;
            syncModel();

            model_->optimize();

            int status = model_->get(GRB_IntAttr_Status);
            int solCount = model_->get(GRB_IntAttr_SolCount);
            solved_ = solCount > 0;

            switch (status) {
                case GRB_OPTIMAL:
                    return MIPStatus::Optimal;
                case GRB_INFEASIBLE:
                    return MIPStatus::Infeasible;
                case GRB_UNBOUNDED:
                    return MIPStatus::Unbounded;
                case GRB_INF_OR_UNBD:
                    return MIPStatus::InfeasibleOrUnbounded;
                case GRB_TIME_LIMIT:
                    return MIPStatus::TimeLimit;
                case GRB_NODE_LIMIT:
                    return MIPStatus::NodeLimit;
                case GRB_SOLUTION_LIMIT:
                    return MIPStatus::SolutionLimit;
                case GRB_SUBOPTIMAL:
                    return MIPStatus::Feasible;
                case GRB_INTERRUPTED:
                    return solved_ ? MIPStatus::Feasible : MIPStatus::Error;
                default:
                    return solved_ ? MIPStatus::Feasible : MIPStatus::Unknown;
            }
        } catch (const GRBException&) {
            solved_ = false;
            return MIPStatus::Error;
        } catch (...) {
            solved_ = false;
            return MIPStatus::Error;
        }
    }

    // ========== Solution Access ==========

    double getValue(Var var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return vars_.at(var.id()).get(GRB_DoubleAttr_X);
    }

    double getObjectiveValue() const override {
        if (!solved_) return std::numeric_limits<double>::infinity();
        try {
            return model_->get(GRB_DoubleAttr_ObjVal);
        } catch (...) {
            return std::numeric_limits<double>::infinity();
        }
    }

    double getObjectiveBound() const override {
        if (!solved_) return 0.0;
        try {
            return model_->get(GRB_DoubleAttr_ObjBound);
        } catch (...) {
            return 0.0;
        }
    }

    double getGap() const override {
        if (!solved_) return 1.0;
        try {
            return model_->get(GRB_DoubleAttr_MIPGap);
        } catch (...) {
            return IMIPBackend::getGap();
        }
    }

    double getReducedCost(Var var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return vars_.at(var.id()).get(GRB_DoubleAttr_RC);
    }

    double getDual(int constraintIndex) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        if (constraintIndex >= static_cast<int>(constraints_.size())) {
            throw std::out_of_range("Invalid constraint index");
        }
        return constraints_.at(constraintIndex).get(GRB_DoubleAttr_Pi);
    }

    // ========== Statistics ==========

    double getSolveTime() const override {
        return model_->get(GRB_DoubleAttr_Runtime);
    }

    long long getNumIterations() const override {
        return static_cast<long long>(model_->get(GRB_DoubleAttr_IterCount));
    }

    long long getNumNodes() const override {
        return static_cast<long long>(model_->get(GRB_DoubleAttr_NodeCount));
    }

    int getNumSolutions() const override {
        return model_->get(GRB_IntAttr_SolCount);
    }

    // ========== Parameters ==========

    void setTimeLimit(double seconds) override {
        time_limit_ = seconds;
        model_->set(GRB_DoubleParam_TimeLimit, seconds);
    }

    void setGapTolerance(double gap) override {
        model_->set(GRB_DoubleParam_MIPGap, gap);
    }

    void setNumThreads(int threads) override {
        model_->set(GRB_IntParam_Threads, threads);
    }

    void setRandomSeed(int seed) override {
        model_->set(GRB_IntParam_Seed, seed);
    }

    void setVerbosity(int level) override {
        output_flag_ = level > 0 ? 1 : 0;
        model_->set(GRB_IntParam_OutputFlag, output_flag_);
    }

    void setPresolve(bool enable) override {
        model_->set(GRB_IntParam_Presolve, enable ? 1 : 0);
    }

    void setNodeLimit(long long limit) override {
        model_->set(GRB_DoubleParam_NodeLimit, static_cast<double>(limit));
    }

    void setSolutionLimit(int limit) override {
        model_->set(GRB_IntParam_SolutionLimit, limit);
    }

    // ========== Model Management ==========

    void clear() override {
        vars_.clear();
        varTypes_.clear();
        constraints_.clear();
        genConstraints_.clear();
        sos_.clear();
        constraintCount_ = 0;
        solved_ = false;
        model_dirty_ = false;

        model_ = std::make_unique<GRBModel>(env_);
        model_->set(GRB_IntParam_OutputFlag, output_flag_);
        model_->set(GRB_DoubleParam_TimeLimit, time_limit_);
    }

    int getNumVars() const override {
        return static_cast<int>(vars_.size());
    }

    int getNumConstraints() const override {
        return constraintCount_;
    }

    long long getNumNonZeros() const override {
        try {
            return static_cast<long long>(model_->get(GRB_IntAttr_NumNZs));
        } catch (...) {
            return 0;
        }
    }

    bool exportModel(const std::string& filename) const override {
        try {
            syncModel();
            model_->write(filename);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool importModel(const std::string& filename) override {
        try {
            model_ = std::make_unique<GRBModel>(env_, filename);
            model_->set(GRB_IntParam_OutputFlag, output_flag_);
            model_->set(GRB_DoubleParam_TimeLimit, time_limit_);
            solved_ = false;
            model_dirty_ = false;
            refreshModelCaches();
            return true;
        } catch (...) {
            return false;
        }
    }

    // ========== Advanced ==========

    MIPStatus solveRelaxation() override {
        std::vector<char> savedTypes = varTypes_;
        for (size_t i = 0; i < vars_.size(); ++i) {
            if (varTypes_[i] != GRB_CONTINUOUS) {
                vars_[i].set(GRB_CharAttr_VType, GRB_CONTINUOUS);
                varTypes_[i] = GRB_CONTINUOUS;
            }
        }

        markDirty();
        MIPStatus status = solve(time_limit_);

        for (size_t i = 0; i < vars_.size(); ++i) {
            if (varTypes_[i] != savedTypes[i]) {
                vars_[i].set(GRB_CharAttr_VType, savedTypes[i]);
            }
        }
        varTypes_ = savedTypes;
        markDirty();

        return status;
    }

    MIPStatus solveFixed() override {
        if (!solved_) {
            throw std::runtime_error("Must have a solution to fix variables");
        }

        std::vector<double> savedLB(vars_.size());
        std::vector<double> savedUB(vars_.size());
        std::vector<char> savedTypes = varTypes_;

        for (size_t i = 0; i < vars_.size(); ++i) {
            savedLB[i] = vars_[i].get(GRB_DoubleAttr_LB);
            savedUB[i] = vars_[i].get(GRB_DoubleAttr_UB);
            if (varTypes_[i] != GRB_CONTINUOUS) {
                double value = vars_[i].get(GRB_DoubleAttr_X);
                vars_[i].set(GRB_DoubleAttr_LB, value);
                vars_[i].set(GRB_DoubleAttr_UB, value);
                vars_[i].set(GRB_CharAttr_VType, GRB_CONTINUOUS);
                varTypes_[i] = GRB_CONTINUOUS;
            }
        }

        markDirty();
        MIPStatus status = solve(time_limit_);

        for (size_t i = 0; i < vars_.size(); ++i) {
            vars_[i].set(GRB_DoubleAttr_LB, savedLB[i]);
            vars_[i].set(GRB_DoubleAttr_UB, savedUB[i]);
            if (varTypes_[i] != savedTypes[i]) {
                vars_[i].set(GRB_CharAttr_VType, savedTypes[i]);
            }
        }
        varTypes_ = savedTypes;
        markDirty();

        return status;
    }

private:
    GRBEnv env_;
    std::unique_ptr<GRBModel> model_;
    bool solved_;
    double time_limit_;
    int constraintCount_;
    mutable bool model_dirty_;
    int output_flag_;

    std::vector<GRBVar> vars_;
    std::vector<char> varTypes_;
    std::vector<GRBConstr> constraints_;
    std::vector<GRBGenConstr> genConstraints_;
    std::vector<GRBSOS> sos_;

    void markDirty() { model_dirty_ = true; }

    void syncModel() const {
        if (!model_dirty_) return;
        model_->update();
        model_dirty_ = false;
    }

    void refreshModelCaches() {
        vars_.clear();
        varTypes_.clear();
        constraints_.clear();
        genConstraints_.clear();
        sos_.clear();
        constraintCount_ = 0;

        int numVars = model_->get(GRB_IntAttr_NumVars);
        std::unique_ptr<GRBVar[]> modelVars(model_->getVars());
        for (int i = 0; i < numVars; ++i) {
            vars_.push_back(modelVars[i]);
            varTypes_.push_back(modelVars[i].get(GRB_CharAttr_VType));
        }

        int numConstrs = model_->get(GRB_IntAttr_NumConstrs);
        std::unique_ptr<GRBConstr[]> modelConstrs(model_->getConstrs());
        for (int i = 0; i < numConstrs; ++i) {
            constraints_.push_back(modelConstrs[i]);
        }

        int numGen = 0;
        try {
            numGen = model_->get(GRB_IntAttr_NumGenConstrs);
        } catch (...) {
            numGen = 0;
        }

        constraintCount_ = numConstrs + numGen;
    }

    GRBLinExpr toGRBLinExpr(const LinearExpr& expr) const {
        GRBLinExpr result(expr.constant());
        for (const auto& [varId, coeff] : expr.terms()) {
            result += coeff * vars_.at(varId);
        }
        return result;
    }

    GRBQuadExpr toGRBQuadExpr(const QuadExpr& expr) const {
        GRBQuadExpr quad(toGRBLinExpr(expr.linearPart()));
        for (const auto& [i, j, coeff] : expr.quadTerms()) {
            quad.addTerm(coeff, vars_.at(i), vars_.at(j));
        }
        return quad;
    }

    char toGRBSense(Sense sense) const {
        switch (sense) {
            case Sense::LessEqual:
                return GRB_LESS_EQUAL;
            case Sense::Equal:
                return GRB_EQUAL;
            case Sense::GreaterEqual:
                return GRB_GREATER_EQUAL;
        }
        return GRB_EQUAL;
    }

    GRBVar getVar(BoolVar var) const {
        return vars_.at(var.id());
    }

    void addLinearConstraint(const LinearExpr& expr, char sense, double rhs,
                             const std::string& name) {
        GRBLinExpr grb = toGRBLinExpr(expr);
        GRBConstr constr = model_->addConstr(grb, sense, rhs, name);
        constraints_.push_back(constr);
        constraintCount_++;
        markDirty();
    }

    void addSOS(const std::vector<Var>& vars,
                const std::vector<double>& weights,
                int type,
                const std::string& name) {
        (void)name;
        if (vars.empty()) {
            return;
        }

        std::vector<GRBVar> grbVars;
        std::vector<double> grbWeights;
        grbVars.reserve(vars.size());
        grbWeights.reserve(vars.size());

        for (size_t i = 0; i < vars.size(); ++i) {
            grbVars.push_back(vars_.at(vars[i].id()));
            if (weights.empty()) {
                grbWeights.push_back(static_cast<double>(i));
            } else {
                grbWeights.push_back(weights[i]);
            }
        }

        GRBSOS sos = model_->addSOS(grbVars.data(), grbWeights.data(),
                                    static_cast<int>(grbVars.size()), type);
        sos_.push_back(sos);
        markDirty();
    }
};

#else // !GUROBI_FOUND

/**
 * @brief Stub implementation when Gurobi is not available
 */
class GurobiMIPBackend : public IMIPBackend {
public:
    GurobiMIPBackend() {
        throw std::runtime_error("Gurobi is not available. Please install Gurobi.");
    }

    std::string name() const override { return "gurobi"; }

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

#endif // GUROBI_FOUND

} // namespace mip
} // namespace routing
