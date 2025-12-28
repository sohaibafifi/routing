// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IMIPBackend.hpp"

#ifdef CPLEX_FOUND
#include <ilcplex/ilocplex.h>
#endif

#include <stdexcept>
#include <cmath>

namespace routing {
namespace mip {

#ifdef CPLEX_FOUND

/**
 * @brief CPLEX implementation of IMIPBackend
 *
 * Uses IBM ILOG CPLEX for mixed integer programming.
 */
class CPLEXMIPBackend : public IMIPBackend {
public:
    CPLEXMIPBackend()
        : env_()
        , model_(env_)
        , cplex_(model_)
        , objective_(env_)
        , solved_(false)
        , constraintCount_(0)
    {
        // Default parameters
        cplex_.setParam(IloCplex::Param::MIP::Display, 0);
    }

    ~CPLEXMIPBackend() override {
        env_.end();
    }

    std::string name() const override { return "cplex"; }

    // ========== Variable Creation ==========

    Var newVar(double lb, double ub, const std::string& name) override {
        int id = static_cast<int>(vars_.size());
        IloNumVar var(env_, lb, ub, ILOFLOAT, name.empty() ? nullptr : name.c_str());
        vars_.push_back(var);
        model_.add(var);
        return Var(id);
    }

    IntVar newIntVar(int lb, int ub, const std::string& name) override {
        int id = static_cast<int>(vars_.size());
        IloNumVar var(env_, lb, ub, ILOINT, name.empty() ? nullptr : name.c_str());
        vars_.push_back(var);
        model_.add(var);
        return IntVar(id);
    }

    BoolVar newBoolVar(const std::string& name) override {
        int id = static_cast<int>(vars_.size());
        IloNumVar var(env_, 0, 1, ILOBOOL, name.empty() ? nullptr : name.c_str());
        vars_.push_back(var);
        model_.add(var);
        return BoolVar(id);
    }

    // ========== Constraint Addition ==========

    void addLessEqual(const LinearExpr& expr, double rhs,
                      const std::string& name) override {
        IloExpr iloExpr = toIloExpr(expr);
        IloRange range(env_, -IloInfinity, iloExpr, rhs, name.empty() ? nullptr : name.c_str());
        model_.add(range);
        constraints_.push_back(range);
        ranges_.push_back(range);
        constraintCount_++;
    }

    void addGreaterEqual(const LinearExpr& expr, double rhs,
                         const std::string& name) override {
        IloExpr iloExpr = toIloExpr(expr);
        IloRange range(env_, rhs, iloExpr, IloInfinity, name.empty() ? nullptr : name.c_str());
        model_.add(range);
        constraints_.push_back(range);
        ranges_.push_back(range);
        constraintCount_++;
    }

    void addEqual(const LinearExpr& expr, double rhs,
                  const std::string& name) override {
        IloExpr iloExpr = toIloExpr(expr);
        IloRange range(env_, rhs, iloExpr, rhs, name.empty() ? nullptr : name.c_str());
        model_.add(range);
        constraints_.push_back(range);
        ranges_.push_back(range);
        constraintCount_++;
    }

    void addRange(const LinearExpr& expr, double lb, double ub,
                  const std::string& name) override {
        IloExpr iloExpr = toIloExpr(expr);
        IloRange range(env_, lb, iloExpr, ub, name.empty() ? nullptr : name.c_str());
        model_.add(range);
        constraints_.push_back(range);
        ranges_.push_back(range);
        constraintCount_++;
    }

    void addIndicator(BoolVar indicator, int value,
                      const LinearExpr& expr, Sense sense, double rhs,
                      const std::string& name) override {
        IloNumVar indVar = getIloVar(Var(indicator.id()));
        IloExpr iloExpr = toIloExpr(expr);

        IloConstraint linearCon;
        switch (sense) {
            case Sense::LessEqual:
                linearCon = (iloExpr <= rhs);
                break;
            case Sense::Equal:
                linearCon = (iloExpr == rhs);
                break;
            case Sense::GreaterEqual:
                linearCon = (iloExpr >= rhs);
                break;
        }

        IloConstraint indicator_con = IloIfThen(env_,
            value == 1 ? (indVar == 1) : (indVar == 0),
            linearCon);
        if (!name.empty()) indicator_con.setName(name.c_str());
        model_.add(indicator_con);
        constraints_.push_back(indicator_con);
        constraintCount_++;
    }

    void addSOS1(const std::vector<Var>& vars,
                 const std::vector<double>& weights,
                 const std::string& name) override {
        IloNumVarArray varArray(env_, static_cast<int>(vars.size()));
        IloNumArray weightArray(env_, static_cast<int>(vars.size()));

        for (size_t i = 0; i < vars.size(); ++i) {
            varArray[static_cast<int>(i)] = getIloVar(vars[i]);
            weightArray[static_cast<int>(i)] = weights.empty() ?
                static_cast<double>(i) : weights[i];
        }

        IloSOS1 sos(env_, varArray, weightArray, name.empty() ? nullptr : name.c_str());
        model_.add(sos);
    }

    void addSOS2(const std::vector<Var>& vars,
                 const std::vector<double>& weights,
                 const std::string& name) override {
        IloNumVarArray varArray(env_, static_cast<int>(vars.size()));
        IloNumArray weightArray(env_, static_cast<int>(vars.size()));

        for (size_t i = 0; i < vars.size(); ++i) {
            varArray[static_cast<int>(i)] = getIloVar(vars[i]);
            weightArray[static_cast<int>(i)] = weights.empty() ?
                static_cast<double>(i) : weights[i];
        }

        IloSOS2 sos(env_, varArray, weightArray, name.empty() ? nullptr : name.c_str());
        model_.add(sos);
    }

    void addLazyConstraint(const LinearExpr& expr, Sense sense, double rhs) override {
        // Lazy constraints in CPLEX are typically added via callbacks
        // Here we add them as regular constraints with lazy attribute
        IloExpr iloExpr = toIloExpr(expr);
        IloRange range;

        switch (sense) {
            case Sense::LessEqual:
                range = IloRange(env_, -IloInfinity, iloExpr, rhs);
                break;
            case Sense::Equal:
                range = IloRange(env_, rhs, iloExpr, rhs);
                break;
            case Sense::GreaterEqual:
                range = IloRange(env_, rhs, iloExpr, IloInfinity);
                break;
        }

        lazyConstraints_.add(range);
    }

    void addUserCut(const LinearExpr& expr, Sense sense, double rhs) override {
        IloExpr iloExpr = toIloExpr(expr);
        IloRange range;

        switch (sense) {
            case Sense::LessEqual:
                range = IloRange(env_, -IloInfinity, iloExpr, rhs);
                break;
            case Sense::Equal:
                range = IloRange(env_, rhs, iloExpr, rhs);
                break;
            case Sense::GreaterEqual:
                range = IloRange(env_, rhs, iloExpr, IloInfinity);
                break;
        }

        userCuts_.add(range);
    }

    // ========== Objective ==========

    void minimize(const LinearExpr& expr) override {
        IloExpr obj = toIloExpr(expr);
        objective_ = IloMinimize(env_, obj);
        model_.add(objective_);
    }

    void maximize(const LinearExpr& expr) override {
        IloExpr obj = toIloExpr(expr);
        objective_ = IloMaximize(env_, obj);
        model_.add(objective_);
    }

    void minimizeQuad(const QuadExpr& expr) override {
        IloExpr obj = toIloExpr(expr.linearPart());
        for (const auto& [i, j, coeff] : expr.quadTerms()) {
            obj += coeff * vars_[i] * vars_[j];
        }
        objective_ = IloMinimize(env_, obj);
        model_.add(objective_);
    }

    void maximizeQuad(const QuadExpr& expr) override {
        IloExpr obj = toIloExpr(expr.linearPart());
        for (const auto& [i, j, coeff] : expr.quadTerms()) {
            obj += coeff * vars_[i] * vars_[j];
        }
        objective_ = IloMaximize(env_, obj);
        model_.add(objective_);
    }

    // ========== Warm Starting ==========

    void setWarmStart(const std::vector<Var>& vars,
                      const std::vector<double>& values) override {
        IloNumVarArray varArray(env_);
        IloNumArray valArray(env_);

        for (size_t i = 0; i < vars.size(); ++i) {
            varArray.add(getIloVar(vars[i]));
            valArray.add(values[i]);
        }

        cplex_.addMIPStart(varArray, valArray);
    }

    void setMIPStart(const std::vector<std::pair<Var, double>>& solution) override {
        IloNumVarArray varArray(env_);
        IloNumArray valArray(env_);

        for (const auto& [var, value] : solution) {
            varArray.add(getIloVar(var));
            valArray.add(value);
        }

        cplex_.addMIPStart(varArray, valArray);
    }

    // ========== Solving ==========

    MIPStatus solve(double timeout) override {
        cplex_.setParam(IloCplex::Param::TimeLimit, timeout);

        // Add lazy constraints if any
        if (lazyConstraints_.getSize() > 0) {
            cplex_.addLazyConstraints(lazyConstraints_);
        }

        // Add user cuts if any
        if (userCuts_.getSize() > 0) {
            cplex_.addUserCuts(userCuts_);
        }

        try {
            cplex_.resetTime();
            if (cplex_.solve()) {
                solved_ = true;
                auto status = cplex_.getStatus();
                if (status == IloAlgorithm::Optimal) {
                    return MIPStatus::Optimal;
                }
                return MIPStatus::Feasible;
            } else {
                solved_ = false;
                auto status = cplex_.getStatus();
                auto cplexStatus = cplex_.getCplexStatus();

                if (status == IloAlgorithm::Infeasible) {
                    return MIPStatus::Infeasible;
                }
                if (status == IloAlgorithm::Unbounded) {
                    return MIPStatus::Unbounded;
                }
                if (cplexStatus == IloCplex::CplexStatus::AbortTimeLim) {
                    return MIPStatus::TimeLimit;
                }
                if (cplexStatus == IloCplex::CplexStatus::AbortItLim ||
                    cplexStatus == IloCplex::CplexStatus::NodeLimFeas ||
                    cplexStatus == IloCplex::CplexStatus::NodeLimInfeas) {
                    return MIPStatus::NodeLimit;
                }
                if (cplexStatus == IloCplex::CplexStatus::SolLim) {
                    return MIPStatus::SolutionLimit;
                }
                return MIPStatus::Unknown;
            }
        } catch (IloException&) {
            return MIPStatus::Error;
        }
    }

    // ========== Solution Access ==========

    double getValue(Var var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return cplex_.getValue(getIloVar(var));
    }

    double getObjectiveValue() const override {
        if (!solved_) return std::numeric_limits<double>::infinity();
        try {
            return cplex_.getObjValue();
        } catch (...) {
            return std::numeric_limits<double>::infinity();
        }
    }

    double getObjectiveBound() const override {
        if (!solved_) return 0.0;
        try {
            return cplex_.getBestObjValue();
        } catch (...) {
            return 0.0;
        }
    }

    double getGap() const override {
        if (!solved_) return 1.0;
        try {
            return cplex_.getMIPRelativeGap();
        } catch (...) {
            return IMIPBackend::getGap();
        }
    }

    double getReducedCost(Var var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return cplex_.getReducedCost(getIloVar(var));
    }

    double getDual(int constraintIndex) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        if (constraintIndex >= static_cast<int>(ranges_.size())) {
            throw std::out_of_range("Invalid constraint index");
        }
        return cplex_.getDual(ranges_[constraintIndex]);
    }

    // ========== Statistics ==========

    double getSolveTime() const override {
        return cplex_.getTime();
    }

    long long getNumIterations() const override {
        return static_cast<long long>(cplex_.getNiterations64());
    }

    long long getNumNodes() const override {
        return static_cast<long long>(cplex_.getNnodes64());
    }

    int getNumSolutions() const override {
        return cplex_.getSolnPoolNsolns();
    }

    // ========== Parameters ==========

    void setTimeLimit(double seconds) override {
        cplex_.setParam(IloCplex::Param::TimeLimit, seconds);
    }

    void setGapTolerance(double gap) override {
        cplex_.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, gap);
    }

    void setNumThreads(int threads) override {
        cplex_.setParam(IloCplex::Param::Threads, threads);
    }

    void setRandomSeed(int seed) override {
        cplex_.setParam(IloCplex::Param::RandomSeed, seed);
    }

    void setVerbosity(int level) override {
        cplex_.setParam(IloCplex::Param::MIP::Display, level);
        cplex_.setParam(IloCplex::Param::Simplex::Display, level > 0 ? 1 : 0);
    }

    void setPresolve(bool enable) override {
        cplex_.setParam(IloCplex::Param::Preprocessing::Presolve, enable);
    }

    void setNodeLimit(long long limit) override {
        cplex_.setParam(IloCplex::Param::MIP::Limits::Nodes, limit);
    }

    void setSolutionLimit(int limit) override {
        cplex_.setParam(IloCplex::Param::MIP::Limits::Solutions, limit);
    }

    // ========== Model Management ==========

    void clear() override {
        vars_.clear();
        constraints_.clear();
        ranges_.clear();
        lazyConstraints_.clear();
        userCuts_.clear();
        model_.end();
        cplex_.end();
        objective_.end();
        model_ = IloModel(env_);
        cplex_ = IloCplex(model_);
        lazyConstraints_ = IloRangeArray(env_);
        userCuts_ = IloRangeArray(env_);
        solved_ = false;
        constraintCount_ = 0;
    }

    int getNumVars() const override {
        return static_cast<int>(vars_.size());
    }

    int getNumConstraints() const override {
        return constraintCount_;
    }

    long long getNumNonZeros() const override {
        return cplex_.getNNZs64();
    }

    bool exportModel(const std::string& filename) const override {
        try {
            cplex_.exportModel(filename.c_str());
            return true;
        } catch (...) {
            return false;
        }
    }

    bool importModel(const std::string& filename) override {
        try {
            cplex_.importModel(model_, filename.c_str());
            return true;
        } catch (...) {
            return false;
        }
    }

    // ========== Advanced ==========

    MIPStatus solveRelaxation() override {
        // Temporarily convert integer/binary variables to continuous
        std::vector<IloConversion> conversions;
        for (const auto& var : vars_) {
            if (var.getType() != ILOFLOAT) {
                IloConversion conv(env_, var, ILOFLOAT);
                model_.add(conv);
                conversions.push_back(conv);
            }
        }

        auto status = solve(cplex_.getParam(IloCplex::Param::TimeLimit));

        // Remove conversions to restore variable types
        for (auto& conv : conversions) {
            model_.remove(conv);
        }
        return status;
    }

    MIPStatus solveFixed() override {
        // Fix integer variables to their solution values
        if (!solved_) {
            throw std::runtime_error("Must have a solution to fix variables");
        }

        IloNumArray vals(env_);
        IloNumVarArray intVars(env_);

        for (const auto& var : vars_) {
            if (var.getType() == ILOINT || var.getType() == ILOBOOL) {
                intVars.add(var);
                vals.add(cplex_.getValue(var));
            }
        }

        // Fix bounds temporarily
        IloNumArray origLB(env_), origUB(env_);
        for (int i = 0; i < intVars.getSize(); ++i) {
            origLB.add(intVars[i].getLB());
            origUB.add(intVars[i].getUB());
            intVars[i].setBounds(vals[i], vals[i]);
        }

        // Convert to continuous
        IloConversion conversion(env_, intVars, ILOFLOAT);
        model_.add(conversion);

        auto status = solve(cplex_.getParam(IloCplex::Param::TimeLimit));

        // Restore
        model_.remove(conversion);
        for (int i = 0; i < intVars.getSize(); ++i) {
            intVars[i].setBounds(origLB[i], origUB[i]);
        }

        return status;
    }

    // ========== CPLEX-Specific Access ==========

    /**
     * @brief Get the underlying CPLEX object for advanced usage
     */
    IloCplex& getCplex() { return cplex_; }
    const IloCplex& getCplex() const { return cplex_; }

    /**
     * @brief Get the underlying model
     */
    IloModel& getModel() { return model_; }
    const IloModel& getModel() const { return model_; }

    /**
     * @brief Get the underlying environment
     */
    IloEnv& getEnv() { return env_; }

    /**
     * @brief Get IloNumVar by handle
     */
    IloNumVar getIloVar(Var var) const {
        return vars_.at(var.id());
    }

private:
    IloEnv env_;
    IloModel model_;
    IloCplex cplex_;
    IloObjective objective_;
    bool solved_;
    int constraintCount_;

    std::vector<IloNumVar> vars_;
    std::vector<IloConstraint> constraints_;
    std::vector<IloRange> ranges_;  // For dual value access
    IloRangeArray lazyConstraints_{env_};
    IloRangeArray userCuts_{env_};

    IloExpr toIloExpr(const LinearExpr& expr) const {
        IloExpr result(env_, expr.constant());
        for (const auto& [varId, coeff] : expr.terms()) {
            result += coeff * vars_[varId];
        }
        return result;
    }
};

#else // !CPLEX_FOUND

/**
 * @brief Stub implementation when CPLEX is not available
 */
class CPLEXMIPBackend : public IMIPBackend {
public:
    CPLEXMIPBackend() {
        throw std::runtime_error("CPLEX is not available. Please install IBM ILOG CPLEX.");
    }

    std::string name() const override { return "cplex"; }

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

#endif // CPLEX_FOUND

} // namespace mip
} // namespace routing
