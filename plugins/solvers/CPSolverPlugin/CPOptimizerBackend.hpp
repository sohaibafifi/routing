// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPBackend.hpp"

#ifdef CPLEX_FOUND
#include <ilcp/cp.h>
#endif

#include <stdexcept>
#include <unordered_map>

namespace routing {
namespace cp {

#ifdef CPLEX_FOUND

/**
 * @brief CP Optimizer implementation of ICPBackend
 *
 * Uses IBM ILOG CP Optimizer for constraint programming.
 */
class CPOptimizerBackend : public ICPBackend {
public:
    CPOptimizerBackend()
        : env_()
        , model_(env_)
        , cp_(model_)
        , objective_(env_)
        , objectiveSense_(IloObjective::Minimize)
        , solved_(false)
    {
        // Set default parameters
        // cp_.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    }

    ~CPOptimizerBackend() override {
        env_.end();
    }

    std::string name() const override { return "cpoptimizer"; }

    // ========== Variable Creation ==========

    IntVar newIntVar(int lb, int ub, const std::string& name) override {
        int id = static_cast<int>(intVars_.size());
        IloIntVar var(env_, lb, ub, name.empty() ? nullptr : name.c_str());
        intVars_.push_back(var);
        return IntVar(id);
    }

    IntVar newBoolVar(const std::string& name) override {
        return newIntVar(0, 1, name);
    }

    IntVar newConstant(int value) override {
        int id = static_cast<int>(intVars_.size());
        IloIntVar var(env_, value, value);
        intVars_.push_back(var);
        return IntVar(id);
    }

    IntervalVar newIntervalVar(int minStart, int maxEnd,
                               int minDuration, int maxDuration,
                               const std::string& name) override {
        int id = static_cast<int>(intervalVars_.size());
        IloIntervalVar var(env_, minDuration, name.empty() ? nullptr : name.c_str());
        var.setStartMin(minStart);
        var.setStartMax(maxEnd - minDuration);
        var.setEndMin(minStart + minDuration);
        var.setEndMax(maxEnd);
        var.setSizeMin(minDuration);
        var.setSizeMax(maxDuration);
        intervalVars_.push_back(var);
        return IntervalVar(id);
    }

    OptionalIntervalVar newOptionalIntervalVar(int minStart, int maxEnd,
                                               int minDuration, int maxDuration,
                                               const std::string& name) override {
        int id = static_cast<int>(optionalIntervalVars_.size());

        // Create presence variable
        IntVar presence = newBoolVar(name + "_presence");

        IloIntervalVar var(env_, minDuration, name.empty() ? nullptr : name.c_str());
        var.setOptional();
        var.setStartMin(minStart);
        var.setStartMax(maxEnd - minDuration);
        var.setEndMin(minStart + minDuration);
        var.setEndMax(maxEnd);
        var.setSizeMin(minDuration);
        var.setSizeMax(maxDuration);
        optionalIntervalVars_.push_back(var);

        // Link presence variable to interval presence
        model_.add(IloPresenceOf(env_, var) == getIloIntVar(presence));

        return OptionalIntervalVar(id, presence);
    }

    // ========== Basic Constraints ==========

    void addEquality(const LinearExpr& expr, int value) override {
        model_.add(toIloExpr(expr) == value);
    }

    void addEquality(IntVar var1, IntVar var2) override {
        model_.add(getIloIntVar(var1) == getIloIntVar(var2));
    }

    void addLinearConstraint(const LinearExpr& expr, int lb, int ub) override {
        IloExpr iloExpr = toIloExpr(expr);
        if (lb > std::numeric_limits<int>::min() / 2) {
            model_.add(iloExpr >= lb);
        }
        if (ub < std::numeric_limits<int>::max() / 2) {
            model_.add(iloExpr <= ub);
        }
    }

    void addLessOrEqual(IntVar var1, IntVar var2, int offset) override {
        model_.add(getIloIntVar(var1) <= getIloIntVar(var2) + offset);
    }

    void addImplication(IntVar condition, const LinearExpr& expr, int lb, int ub) override {
        IloExpr iloExpr = toIloExpr(expr);
        IloIntVar cond = getIloIntVar(condition);
        if (lb > std::numeric_limits<int>::min() / 2) {
            model_.add(IloIfThen(env_, cond == 1, iloExpr >= lb));
        }
        if (ub < std::numeric_limits<int>::max() / 2) {
            model_.add(IloIfThen(env_, cond == 1, iloExpr <= ub));
        }
    }

    void addReification(IntVar indicator, IntVar var, int value) override {
        // Create equivalence: indicator == 1 iff var == value
        IloIntVar ind = getIloIntVar(indicator);
        IloIntVar v = getIloIntVar(var);
        model_.add((v == value) == (ind == 1));
    }

    // ========== Global Constraints ==========

    void addAllDifferent(const std::vector<IntVar>& vars) override {
        IloIntVarArray arr(env_, static_cast<int>(vars.size()));
        for (size_t i = 0; i < vars.size(); ++i) {
            arr[static_cast<int>(i)] = getIloIntVar(vars[i]);
        }
        model_.add(IloAllDiff(env_, arr));
    }

    void addElement(IntVar index, const std::vector<int>& array, IntVar result) override {
        IloIntArray values(env_, static_cast<int>(array.size()));
        for (size_t i = 0; i < array.size(); ++i) {
            values[static_cast<int>(i)] = array[i];
        }
        model_.add(IloElement(values, getIloIntVar(index)) == getIloIntVar(result));
    }

    void addElement(IntVar index, const std::vector<IntVar>& vars, IntVar result) override {
        IloIntVarArray arr(env_, static_cast<int>(vars.size()));
        for (size_t i = 0; i < vars.size(); ++i) {
            arr[static_cast<int>(i)] = getIloIntVar(vars[i]);
        }
        model_.add(IloElement(arr, getIloIntVar(index)) == getIloIntVar(result));
    }

    void addCircuit(const std::vector<IntVar>& next) override {
        // CP Optimizer doesn't have a direct circuit constraint
        // We need to implement it using subcircuit or manual constraints
        int n = static_cast<int>(next.size());

        // All-different on successor variables
        addAllDifferent(next);

        // Use position variables to prevent subtours
        std::vector<IntVar> pos;
        for (int i = 0; i < n; ++i) {
            pos.push_back(newIntVar(0, n - 1, "pos_" + std::to_string(i)));
        }

        // pos[next[i]] = pos[i] + 1 (mod n) for all i except where next[i] = start
        // This is a simplified MTZ-like formulation
        IntVar startNode = newConstant(0);
        addEquality(LinearExpr(pos[0]), 0);

        for (int i = 0; i < n; ++i) {
            // If next[i] != 0, then pos[next[i]] = pos[i] + 1
            // This is approximated; full circuit requires more constraints
            for (int j = 1; j < n; ++j) {
                // If next[i] = j, then pos[j] = pos[i] + 1
                IntVar isNext = newBoolVar("");
                model_.add((getIloIntVar(next[i]) == j) == (getIloIntVar(isNext) == 1));
                model_.add(IloIfThen(env_, getIloIntVar(isNext) == 1,
                                     getIloIntVar(pos[j]) == getIloIntVar(pos[i]) + 1));
            }
        }
    }

    void addSubCircuit(const std::vector<IntVar>& next) override {
        // SubCircuit allows self-loops (next[i] = i means node i is not used)
        // Nodes that are not self-loops must form valid cycles
        //
        // Unlike circuit (which requires all nodes form a single Hamiltonian cycle),
        // subcircuit allows:
        // 1. Self-loops (node points to itself - not used in any tour)
        // 2. Multiple disjoint cycles (each vehicle has its own tour)
        //
        // CP Optimizer doesn't have native subcircuit support.
        // We implement the "no two nodes point to same target" constraint,
        // excluding self-loops.

        int n = static_cast<int>(next.size());

        // For each pair of nodes, they cannot both point to the same non-self target
        // Use element constraint approach: create inverse variables
        // prev[j] = i means node i points to j (next[i] = j)
        // Each non-self-loop target can have at most one predecessor

        // Simpler approach: for each possible target k,
        // at most one node i (where i != k) can have next[i] = k
        for (int k = 0; k < n; ++k) {
            IloIntVarArray pointsToK(env_);
            for (int i = 0; i < n; ++i) {
                if (i != k) {  // Exclude self-loop case
                    IloIntVar indicator = IloBoolVar(env_);
                    model_.add((getIloIntVar(next[i]) == k) == (indicator == 1));
                    pointsToK.add(indicator);
                }
            }
            // At most one node can point to k (excluding self-loop)
            if (pointsToK.getSize() > 0) {
                model_.add(IloSum(pointsToK) <= 1);
            }
        }
    }

    void addInverse(const std::vector<IntVar>& next,
                   const std::vector<IntVar>& prev) override {
        IloIntVarArray nextArr(env_, static_cast<int>(next.size()));
        IloIntVarArray prevArr(env_, static_cast<int>(prev.size()));
        for (size_t i = 0; i < next.size(); ++i) {
            nextArr[static_cast<int>(i)] = getIloIntVar(next[i]);
        }
        for (size_t i = 0; i < prev.size(); ++i) {
            prevArr[static_cast<int>(i)] = getIloIntVar(prev[i]);
        }
        model_.add(IloInverse(env_, nextArr, prevArr));
    }

    // ========== Scheduling Constraints ==========

    void addNoOverlap(const std::vector<IntervalVar>& intervals) override {
        IloIntervalVarArray arr(env_, static_cast<int>(intervals.size()));
        for (size_t i = 0; i < intervals.size(); ++i) {
            arr[static_cast<int>(i)] = getIloIntervalVar(intervals[i]);
        }
        model_.add(IloNoOverlap(env_, arr));
    }

    void addNoOverlap(const std::vector<OptionalIntervalVar>& intervals) override {
        IloIntervalVarArray arr(env_, static_cast<int>(intervals.size()));
        for (size_t i = 0; i < intervals.size(); ++i) {
            arr[static_cast<int>(i)] = getIloOptionalIntervalVar(intervals[i]);
        }
        model_.add(IloNoOverlap(env_, arr));
    }

    void addCumulative(const std::vector<IntervalVar>& intervals,
                      const std::vector<int>& demands,
                      int capacity) override {
        IloCumulFunctionExpr cumul(env_);
        for (size_t i = 0; i < intervals.size(); ++i) {
            cumul += IloPulse(getIloIntervalVar(intervals[i]), demands[i]);
        }
        model_.add(cumul <= capacity);
    }

    void addCumulative(const std::vector<OptionalIntervalVar>& intervals,
                      const std::vector<int>& demands,
                      int capacity) override {
        IloCumulFunctionExpr cumul(env_);
        for (size_t i = 0; i < intervals.size(); ++i) {
            cumul += IloPulse(getIloOptionalIntervalVar(intervals[i]), demands[i]);
        }
        model_.add(cumul <= capacity);
    }

    void addEndBeforeStart(IntervalVar interval1, IntervalVar interval2,
                          int delay) override {
        model_.add(IloEndBeforeStart(env_,
                                     getIloIntervalVar(interval1),
                                     getIloIntervalVar(interval2),
                                     delay));
    }

    IntVar startOf(IntervalVar interval) override {
        // Create a new integer variable linked to the start
        int id = static_cast<int>(intVars_.size());
        IloIntVar startVar(env_, IloIntMin, IloIntMax);
        intVars_.push_back(startVar);
        model_.add(startVar == IloStartOf(getIloIntervalVar(interval)));
        return IntVar(id);
    }

    IntVar endOf(IntervalVar interval) override {
        int id = static_cast<int>(intVars_.size());
        IloIntVar endVar(env_, IloIntMin, IloIntMax);
        intVars_.push_back(endVar);
        model_.add(endVar == IloEndOf(getIloIntervalVar(interval)));
        return IntVar(id);
    }

    IntVar durationOf(IntervalVar interval) override {
        int id = static_cast<int>(intVars_.size());
        IloIntVar durVar(env_, IloIntMin, IloIntMax);
        intVars_.push_back(durVar);
        model_.add(durVar == IloSizeOf(getIloIntervalVar(interval)));
        return IntVar(id);
    }

    IntVar startOf(OptionalIntervalVar interval, int defaultValue) override {
        int id = static_cast<int>(intVars_.size());
        IloIntVar startVar(env_, IloIntMin, IloIntMax);
        intVars_.push_back(startVar);
        model_.add(startVar == IloStartOf(getIloOptionalIntervalVar(interval), defaultValue));
        return IntVar(id);
    }

    IntVar endOf(OptionalIntervalVar interval, int defaultValue) override {
        int id = static_cast<int>(intVars_.size());
        IloIntVar endVar(env_, IloIntMin, IloIntMax);
        intVars_.push_back(endVar);
        model_.add(endVar == IloEndOf(getIloOptionalIntervalVar(interval), defaultValue));
        return IntVar(id);
    }

    // ========== Objective ==========

    void minimize(const LinearExpr& expr) override {
        objectiveExpr_ = expr;
        objectiveSense_ = IloObjective::Minimize;
    }

    void maximize(const LinearExpr& expr) override {
        objectiveExpr_ = expr;
        objectiveSense_ = IloObjective::Maximize;
    }

    void minimizeMakespan(const std::vector<IntervalVar>& intervals) override {
        IloIntExprArray ends(env_, static_cast<int>(intervals.size()));
        for (size_t i = 0; i < intervals.size(); ++i) {
            ends[static_cast<int>(i)] = IloEndOf(getIloIntervalVar(intervals[i]));
        }
        objective_ = IloMinimize(env_, IloMax(ends));
        model_.add(objective_);
    }

    // ========== Solving ==========

    CPStatus solve(double timeout) override {
        // Add objective if set via minimize/maximize
        if (!objectiveExpr_.isEmpty()) {
            IloExpr obj = toIloExpr(objectiveExpr_);
            objective_ = IloObjective(env_, obj, objectiveSense_);
            model_.add(objective_);
        }

        cp_.setParameter(IloCP::TimeLimit, timeout);

        try {
            if (cp_.solve()) {
                solved_ = true;
                auto status = cp_.getStatus();
                if (status == IloAlgorithm::Optimal) {
                    return CPStatus::Optimal;
                }
                return CPStatus::Feasible;
            } else {
                solved_ = false;
                auto status = cp_.getStatus();
                if (status == IloAlgorithm::Infeasible) {
                    return CPStatus::Infeasible;
                }
                return CPStatus::Unknown;
            }
        } catch (IloException& e) {
            return CPStatus::Error;
        }
    }

    void setHint(const std::vector<std::pair<IntVar, int>>& hints) override {
        IloSolution sol(env_);
        for (const auto& [var, value] : hints) {
            sol.setValue(getIloIntVar(var), value);
        }
        cp_.setStartingPoint(sol);
    }

    void setNumWorkers(int workers) override {
        cp_.setParameter(IloCP::Workers, workers);
    }

    void setRandomSeed(int seed) override {
        cp_.setParameter(IloCP::RandomSeed, seed);
    }

    // ========== Solution Access ==========

    int getValue(IntVar var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(cp_.getValue(getIloIntVar(var)));
    }

    bool getBoolValue(IntVar var) const override {
        return getValue(var) != 0;
    }

    int getStart(IntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(cp_.getStart(getIloIntervalVar(interval)));
    }

    int getEnd(IntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(cp_.getEnd(getIloIntervalVar(interval)));
    }

    int getDuration(IntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(cp_.getSize(getIloIntervalVar(interval)));
    }

    bool isPresent(OptionalIntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return cp_.isPresent(getIloOptionalIntervalVar(interval));
    }

    double getObjectiveValue() const override {
        if (!solved_) return std::numeric_limits<double>::infinity();
        // If no objective was set, return 0 (satisfiability problem)
        if (objectiveExpr_.isEmpty()) return 0.0;
        try {
            return cp_.getObjValue();
        } catch (...) {
            return 0.0;  // No objective available
        }
    }

    double getObjectiveBound() const override {
        if (!solved_) return 0.0;
        if (objectiveExpr_.isEmpty()) return 0.0;
        try {
            return cp_.getObjBound();
        } catch (...) {
            return 0.0;
        }
    }

    double getSolveTime() const override {
        return cp_.getInfo(IloCP::SolveTime);
    }

    long long getNumBranches() const override {
        return static_cast<long long>(cp_.getInfo(IloCP::NumberOfBranches));
    }

    long long getNumFailures() const override {
        return static_cast<long long>(cp_.getInfo(IloCP::NumberOfFails));
    }

    void clear() override {
        intVars_.clear();
        intervalVars_.clear();
        optionalIntervalVars_.clear();
        model_.end();
        cp_.end();
        objective_.end();
        model_ = IloModel(env_);
        cp_ = IloCP(model_);
        solved_ = false;
    }

    bool exportModel(const std::string& filename) const override {
        try {
            cp_.exportModel(filename.c_str());
            return true;
        } catch (...) {
            return false;
        }
    }

private:
    IloEnv env_;
    IloModel model_;
    IloCP cp_;
    IloObjective objective_;
    IloObjective::Sense objectiveSense_;
    LinearExpr objectiveExpr_;
    bool solved_;

    std::vector<IloIntVar> intVars_;
    std::vector<IloIntervalVar> intervalVars_;
    std::vector<IloIntervalVar> optionalIntervalVars_;

    IloIntVar getIloIntVar(IntVar var) const {
        return intVars_.at(var.id());
    }

    IloIntervalVar getIloIntervalVar(IntervalVar var) const {
        return intervalVars_.at(var.id());
    }

    IloIntervalVar getIloOptionalIntervalVar(OptionalIntervalVar var) const {
        return optionalIntervalVars_.at(var.id());
    }

    IloExpr toIloExpr(const LinearExpr& expr) const {
        IloExpr result(env_, expr.constant());
        for (const auto& [var, coeff] : expr.terms()) {
            result += coeff * getIloIntVar(var);
        }
        return result;
    }
};

#else // !CPOPTIMIZER_FOUND

/**
 * @brief Stub implementation when CP Optimizer is not available
 */
class CPOptimizerBackend : public ICPBackend {
public:
    CPOptimizerBackend() {
        throw std::runtime_error("CP Optimizer is not available. Please install IBM ILOG CP Optimizer.");
    }

    std::string name() const override { return "cpoptimizer"; }

    IntVar newIntVar(int, int, const std::string&) override { return IntVar(); }
    IntVar newBoolVar(const std::string&) override { return IntVar(); }
    IntVar newConstant(int) override { return IntVar(); }
    IntervalVar newIntervalVar(int, int, int, int, const std::string&) override { return IntervalVar(); }
    OptionalIntervalVar newOptionalIntervalVar(int, int, int, int, const std::string&) override {
        return OptionalIntervalVar();
    }

    void addEquality(const LinearExpr&, int) override {}
    void addEquality(IntVar, IntVar) override {}
    void addLinearConstraint(const LinearExpr&, int, int) override {}
    void addLessOrEqual(IntVar, IntVar, int) override {}
    void addImplication(IntVar, const LinearExpr&, int, int) override {}
    void addReification(IntVar, IntVar, int) override {}

    void addAllDifferent(const std::vector<IntVar>&) override {}
    void addElement(IntVar, const std::vector<int>&, IntVar) override {}
    void addElement(IntVar, const std::vector<IntVar>&, IntVar) override {}
    void addCircuit(const std::vector<IntVar>&) override {}
    void addSubCircuit(const std::vector<IntVar>&) override {}
    void addInverse(const std::vector<IntVar>&, const std::vector<IntVar>&) override {}

    void addNoOverlap(const std::vector<IntervalVar>&) override {}
    void addNoOverlap(const std::vector<OptionalIntervalVar>&) override {}
    void addCumulative(const std::vector<IntervalVar>&, const std::vector<int>&, int) override {}
    void addCumulative(const std::vector<OptionalIntervalVar>&, const std::vector<int>&, int) override {}
    void addEndBeforeStart(IntervalVar, IntervalVar, int) override {}

    IntVar startOf(IntervalVar) override { return IntVar(); }
    IntVar endOf(IntervalVar) override { return IntVar(); }
    IntVar durationOf(IntervalVar) override { return IntVar(); }
    IntVar startOf(OptionalIntervalVar, int) override { return IntVar(); }
    IntVar endOf(OptionalIntervalVar, int) override { return IntVar(); }

    void minimize(const LinearExpr&) override {}
    void maximize(const LinearExpr&) override {}
    void minimizeMakespan(const std::vector<IntervalVar>&) override {}

    CPStatus solve(double) override { return CPStatus::Error; }
    void setHint(const std::vector<std::pair<IntVar, int>>&) override {}
    void setNumWorkers(int) override {}
    void setRandomSeed(int) override {}

    int getValue(IntVar) const override { return 0; }
    bool getBoolValue(IntVar) const override { return false; }
    int getStart(IntervalVar) const override { return 0; }
    int getEnd(IntervalVar) const override { return 0; }
    int getDuration(IntervalVar) const override { return 0; }
    bool isPresent(OptionalIntervalVar) const override { return false; }
    double getObjectiveValue() const override { return 0.0; }
    double getObjectiveBound() const override { return 0.0; }

    double getSolveTime() const override { return 0.0; }
    long long getNumBranches() const override { return 0; }
    long long getNumFailures() const override { return 0; }

    void clear() override {}
};

#endif // CPOPTIMIZER_FOUND

} // namespace cp
} // namespace routing
