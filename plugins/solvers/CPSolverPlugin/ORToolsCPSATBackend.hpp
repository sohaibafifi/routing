// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPBackend.hpp"

#ifdef ORTOOLS_FOUND
#include <fstream>
#include "ortools/base/base_export.h"
// Some OR-Tools installs expect OR_PROTO_DLL to be defined by consumers.
#ifndef OR_PROTO_DLL
#define OR_PROTO_DLL OR_DLL
#endif
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model_solver.h"
#endif

#include <stdexcept>
#include <unordered_map>
#include <chrono>
#include <algorithm>

namespace routing {
namespace cp {

#ifdef ORTOOLS_FOUND

/**
 * @brief OR-Tools CP-SAT implementation of ICPBackend
 *
 * Uses Google OR-Tools CP-SAT solver for constraint programming.
 */
class ORToolsCPSATBackend : public ICPBackend {
public:
    ORToolsCPSATBackend()
        : solved_(false)
        , solve_time_(0.0)
    {
        // Set default parameters
        params_.set_log_search_progress(false);
    }

    ~ORToolsCPSATBackend() override = default;

    std::string name() const override { return "ortools-cpsat"; }

    // ========== Variable Creation ==========

    IntVar newIntVar(int lb, int ub, const std::string& name) override {
        int id = static_cast<int>(intVars_.size());
        auto var = cp_.NewIntVar(operations_research::Domain(lb, ub)).WithName(name);
        intVars_.push_back(var);
        return IntVar(id);
    }

    IntVar newBoolVar(const std::string& name) override {
        int id = static_cast<int>(intVars_.size());
        auto var = cp_.NewBoolVar().WithName(name);
        intVars_.push_back(operations_research::sat::IntVar(var));
        return IntVar(id);
    }

    IntVar newConstant(int value) override {
        int id = static_cast<int>(intVars_.size());
        auto var = cp_.NewConstant(value);
        intVars_.push_back(var);
        return IntVar(id);
    }

    IntervalVar newIntervalVar(int minStart, int maxEnd,
                               int minDuration, int maxDuration,
                               const std::string& name) override {
        int id = static_cast<int>(intervalVars_.size());

        // Create start, end, size variables
        auto start = cp_.NewIntVar(operations_research::Domain(minStart, maxEnd - minDuration));
        auto end = cp_.NewIntVar(operations_research::Domain(minStart + minDuration, maxEnd));
        auto size = cp_.NewIntVar(operations_research::Domain(minDuration, maxDuration));

        // Create interval var
        auto interval = cp_.NewIntervalVar(start, size, end).WithName(name);

        // Store the components
        IntervalComponents comp{start, end, size, interval};
        intervalVars_.push_back(comp);

        return IntervalVar(id);
    }

    OptionalIntervalVar newOptionalIntervalVar(int minStart, int maxEnd,
                                               int minDuration, int maxDuration,
                                               const std::string& name) override {
        int id = static_cast<int>(optionalIntervalVars_.size());

        // Create presence variable
        auto presenceBoolVar = cp_.NewBoolVar().WithName(name + "_presence");
        int presenceId = static_cast<int>(intVars_.size());
        intVars_.push_back(operations_research::sat::IntVar(presenceBoolVar));
        IntVar presence(presenceId);

        // Create start, end, size variables
        auto start = cp_.NewIntVar(operations_research::Domain(minStart, maxEnd - minDuration));
        auto end = cp_.NewIntVar(operations_research::Domain(minStart + minDuration, maxEnd));
        auto size = cp_.NewIntVar(operations_research::Domain(minDuration, maxDuration));

        // Create optional interval var using the BoolVar
        auto interval = cp_.NewOptionalIntervalVar(start, size, end, presenceBoolVar).WithName(name);

        // Store the components
        OptionalIntervalComponents comp{start, end, size, interval, presence};
        optionalIntervalVars_.push_back(comp);

        return OptionalIntervalVar(id, presence);
    }

    // ========== Basic Constraints ==========

    void addEquality(const LinearExpr& expr, int value) override {
        cp_.AddEquality(toOrtLinearExpr(expr), value);
    }

    void addEquality(IntVar var1, IntVar var2) override {
        cp_.AddEquality(getOrtVar(var1), getOrtVar(var2));
    }

    void addLinearConstraint(const LinearExpr& expr, int lb, int ub) override {
        auto ortExpr = toOrtLinearExpr(expr);
        if (lb > std::numeric_limits<int>::min() / 2) {
            cp_.AddGreaterOrEqual(ortExpr, lb);
        }
        if (ub < std::numeric_limits<int>::max() / 2) {
            cp_.AddLessOrEqual(ortExpr, ub);
        }
    }

    void addLessOrEqual(IntVar var1, IntVar var2, int offset) override {
        cp_.AddLessOrEqual(getOrtVar(var1), operations_research::sat::LinearExpr(getOrtVar(var2)) + offset);
    }

    void addImplication(IntVar condition, const LinearExpr& expr, int lb, int ub) override {
        auto cond = getOrtBoolVar(condition);
        auto ortExpr = toOrtLinearExpr(expr);

        if (lb > std::numeric_limits<int>::min() / 2) {
            cp_.AddGreaterOrEqual(ortExpr, lb).OnlyEnforceIf(cond);
        }
        if (ub < std::numeric_limits<int>::max() / 2) {
            cp_.AddLessOrEqual(ortExpr, ub).OnlyEnforceIf(cond);
        }
    }

    void addReification(IntVar indicator, IntVar var, int value) override {
        // Create equivalence: indicator == 1 iff var == value
        auto ind = getOrtBoolVar(indicator);
        auto v = getOrtVar(var);
        cp_.AddEquality(v, value).OnlyEnforceIf(ind);
        cp_.AddNotEqual(v, value).OnlyEnforceIf(ind.Not());
    }

    // ========== Global Constraints ==========

    void addAllDifferent(const std::vector<IntVar>& vars) override {
        std::vector<operations_research::sat::IntVar> ortVars;
        for (const auto& var : vars) {
            ortVars.push_back(getOrtVar(var));
        }
        cp_.AddAllDifferent(ortVars);
    }

    void addElement(IntVar index, const std::vector<int>& array, IntVar result) override {
        std::vector<int64_t> values;
        values.reserve(array.size());
        for (int value : array) {
            values.push_back(static_cast<int64_t>(value));
        }
        cp_.AddElement(getOrtVar(index), values, getOrtVar(result));
    }

    void addElement(IntVar index, const std::vector<IntVar>& vars, IntVar result) override {
        std::vector<operations_research::sat::LinearExpr> ortVars;
        for (const auto& var : vars) {
            ortVars.emplace_back(getOrtVar(var));
        }
        cp_.AddElement(getOrtVar(index), ortVars, getOrtVar(result));
    }

    void addCircuit(const std::vector<IntVar>& next) override {
        // For a Hamiltonian circuit in a complete graph:
        // 1. All next values must be different (it's a permutation)
        // 2. No self-loops
        // 3. No subtours

        int n = static_cast<int>(next.size());

        // All different ensures it's a permutation
        addAllDifferent(next);

        // No self-loops
        for (int i = 0; i < n; ++i) {
            cp_.AddNotEqual(getOrtVar(next[i]), i);
        }

        // Prevent subtours using MTZ-like constraints with position variables
        std::vector<operations_research::sat::IntVar> pos;
        for (int i = 0; i < n; ++i) {
            pos.push_back(cp_.NewIntVar(operations_research::Domain(0, n - 1)));
        }

        // pos[0] = 0 (fix starting point)
        cp_.AddEquality(pos[0], 0);

        // For all i > 0: pos[i] >= 1
        for (int i = 1; i < n; ++i) {
            cp_.AddGreaterOrEqual(pos[i], 1);
        }

        // For all i, j: if next[i] = j, then pos[j] = pos[i] + 1 (mod n)
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j) {
                    auto isNext = cp_.NewBoolVar();
                    cp_.AddEquality(getOrtVar(next[i]), j).OnlyEnforceIf(isNext);
                    cp_.AddNotEqual(getOrtVar(next[i]), j).OnlyEnforceIf(isNext.Not());

                    if (j == 0) {
                        // If next[i] = 0, then pos[i] = n-1 (wrapping back to start)
                        cp_.AddEquality(pos[i], n - 1).OnlyEnforceIf(isNext);
                    } else {
                        // If next[i] = j (j != 0), then pos[j] = pos[i] + 1
                        cp_.AddEquality(pos[j], operations_research::sat::LinearExpr(pos[i]) + 1).OnlyEnforceIf(isNext);
                    }
                }
            }
        }
    }

    void addSubCircuit(const std::vector<IntVar>& next) override {
        // SubCircuit allows self-loops (node points to itself)
        // For each pair of nodes, at most one can point to the same target
        int n = static_cast<int>(next.size());

        for (int k = 0; k < n; ++k) {
            std::vector<operations_research::sat::BoolVar> indicators;
            for (int i = 0; i < n; ++i) {
                if (i != k) {  // Exclude self-loop case
                    auto indicator = cp_.NewBoolVar();
                    cp_.AddEquality(getOrtVar(next[i]), k).OnlyEnforceIf(indicator);
                    cp_.AddNotEqual(getOrtVar(next[i]), k).OnlyEnforceIf(indicator.Not());
                    indicators.push_back(indicator);
                }
            }
            // At most one node can point to k (excluding self-loop)
            if (!indicators.empty()) {
                cp_.AddLessOrEqual(operations_research::sat::LinearExpr::Sum(indicators), 1);
            }
        }
    }

    void addInverse(const std::vector<IntVar>& next,
                   const std::vector<IntVar>& prev) override {
        int n = static_cast<int>(next.size());

        for (int i = 0; i < n; ++i) {
            // For all j: next[i] == j => prev[j] == i
            for (int j = 0; j < n; ++j) {
                auto nextIsJ = cp_.NewBoolVar();
                cp_.AddEquality(getOrtVar(next[i]), j).OnlyEnforceIf(nextIsJ);
                cp_.AddNotEqual(getOrtVar(next[i]), j).OnlyEnforceIf(nextIsJ.Not());

                cp_.AddEquality(getOrtVar(prev[j]), i).OnlyEnforceIf(nextIsJ);
            }
        }
    }

    // ========== Scheduling Constraints ==========

    void addNoOverlap(const std::vector<IntervalVar>& intervals) override {
        std::vector<operations_research::sat::IntervalVar> ortIntervals;
        for (const auto& interval : intervals) {
            ortIntervals.push_back(getOrtIntervalVar(interval));
        }
        cp_.AddNoOverlap(ortIntervals);
    }

    void addNoOverlap(const std::vector<OptionalIntervalVar>& intervals) override {
        std::vector<operations_research::sat::IntervalVar> ortIntervals;
        for (const auto& interval : intervals) {
            ortIntervals.push_back(getOrtOptionalIntervalVar(interval));
        }
        cp_.AddNoOverlap(ortIntervals);
    }

    void addCumulative(const std::vector<IntervalVar>& intervals,
                      const std::vector<int>& demands,
                      int capacity) override {
        auto cumul = cp_.AddCumulative(capacity);
        for (size_t i = 0; i < intervals.size(); ++i) {
            cumul.AddDemand(getOrtIntervalVar(intervals[i]), demands[i]);
        }
    }

    void addCumulative(const std::vector<OptionalIntervalVar>& intervals,
                      const std::vector<int>& demands,
                      int capacity) override {
        auto cumul = cp_.AddCumulative(capacity);
        for (size_t i = 0; i < intervals.size(); ++i) {
            cumul.AddDemand(getOrtOptionalIntervalVar(intervals[i]), demands[i]);
        }
    }

    void addEndBeforeStart(IntervalVar interval1, IntervalVar interval2,
                          int delay) override {
        auto end1 = intervalVars_[interval1.id()].end;
        auto start2 = intervalVars_[interval2.id()].start;
        cp_.AddLessOrEqual(operations_research::sat::LinearExpr(end1) + delay, start2);
    }

    IntVar startOf(IntervalVar interval) override {
        return IntVar(addTrackedVar(intervalVars_[interval.id()].start));
    }

    IntVar endOf(IntervalVar interval) override {
        return IntVar(addTrackedVar(intervalVars_[interval.id()].end));
    }

    IntVar durationOf(IntervalVar interval) override {
        return IntVar(addTrackedVar(intervalVars_[interval.id()].size));
    }

    IntVar startOf(OptionalIntervalVar interval, int /*defaultValue*/) override {
        return IntVar(addTrackedVar(optionalIntervalVars_[interval.id()].start));
    }

    IntVar endOf(OptionalIntervalVar interval, int /*defaultValue*/) override {
        return IntVar(addTrackedVar(optionalIntervalVars_[interval.id()].end));
    }

    // ========== Objective ==========

    void minimize(const LinearExpr& expr) override {
        cp_.Minimize(toOrtLinearExpr(expr));
    }

    void maximize(const LinearExpr& expr) override {
        cp_.Maximize(toOrtLinearExpr(expr));
    }

    void minimizeMakespan(const std::vector<IntervalVar>& intervals) override {
        if (intervals.empty()) {
            return;
        }
        std::vector<operations_research::sat::IntVar> ends;
        ends.reserve(intervals.size());
        int64_t min_end = std::numeric_limits<int64_t>::max();
        int64_t max_end = std::numeric_limits<int64_t>::min();
        for (const auto& interval : intervals) {
            const auto& end_var = intervalVars_[interval.id()].end;
            ends.push_back(end_var);
            const auto domain = end_var.Domain();
            min_end = std::min(min_end, domain.Min());
            max_end = std::max(max_end, domain.Max());
        }
        auto makespan = cp_.NewIntVar(operations_research::Domain(min_end, max_end))
                            .WithName("makespan");
        cp_.AddMaxEquality(makespan, ends);
        cp_.Minimize(makespan);
    }

    // ========== Solving ==========

    CPStatus solve(double timeout) override {
        params_.set_max_time_in_seconds(timeout);

        auto start_time = std::chrono::steady_clock::now();

        // Create a Model for the solver
        operations_research::sat::Model model;
        model.Add(operations_research::sat::NewSatParameters(params_));
        response_ = operations_research::sat::SolveCpModel(cp_.Build(), &model);

        auto end_time = std::chrono::steady_clock::now();
        solve_time_ = std::chrono::duration<double>(end_time - start_time).count();

        solved_ = (response_.status() == operations_research::sat::CpSolverStatus::OPTIMAL ||
                   response_.status() == operations_research::sat::CpSolverStatus::FEASIBLE);

        switch (response_.status()) {
            case operations_research::sat::CpSolverStatus::OPTIMAL:
                return CPStatus::Optimal;
            case operations_research::sat::CpSolverStatus::FEASIBLE:
                return CPStatus::Feasible;
            case operations_research::sat::CpSolverStatus::INFEASIBLE:
                return CPStatus::Infeasible;
            case operations_research::sat::CpSolverStatus::MODEL_INVALID:
                return CPStatus::ModelInvalid;
            default:
                return CPStatus::Unknown;
        }
    }

    void setHint(const std::vector<std::pair<IntVar, int>>& hints) override {
        for (const auto& [var, value] : hints) {
            cp_.AddHint(getOrtVar(var), value);
        }
    }

    void setNumWorkers(int workers) override {
        params_.set_num_workers(workers);
    }

    void setRandomSeed(int seed) override {
        params_.set_random_seed(seed);
    }

    // ========== Solution Access ==========

    int getValue(IntVar var) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(operations_research::sat::SolutionIntegerValue(response_, getOrtVar(var)));
    }

    bool getBoolValue(IntVar var) const override {
        return getValue(var) != 0;
    }

    int getStart(IntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(operations_research::sat::SolutionIntegerValue(
            response_, intervalVars_.at(interval.id()).start));
    }

    int getEnd(IntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(operations_research::sat::SolutionIntegerValue(
            response_, intervalVars_.at(interval.id()).end));
    }

    int getDuration(IntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return static_cast<int>(operations_research::sat::SolutionIntegerValue(
            response_, intervalVars_.at(interval.id()).size));
    }

    bool isPresent(OptionalIntervalVar interval) const override {
        if (!solved_) throw std::runtime_error("No solution available");
        return getBoolValue(interval.presenceVar());
    }

    double getObjectiveValue() const override {
        if (!solved_) return std::numeric_limits<double>::infinity();
        return response_.objective_value();
    }

    double getObjectiveBound() const override {
        if (!solved_) return 0.0;
        return response_.best_objective_bound();
    }

    double getSolveTime() const override {
        return solve_time_;
    }

    long long getNumBranches() const override {
        return response_.num_branches();
    }

    long long getNumFailures() const override {
        return response_.num_conflicts();
    }

    void clear() override {
        cp_ = operations_research::sat::CpModelBuilder();
        intVars_.clear();
        intervalVars_.clear();
        optionalIntervalVars_.clear();
        solved_ = false;
        solve_time_ = 0.0;
    }

    bool exportModel(const std::string& filename) const override {
        try {
            std::ofstream out(filename);
            if (!out) return false;
            out << cp_.Build().DebugString();
            return true;
        } catch (...) {
            return false;
        }
    }

private:
    operations_research::sat::CpModelBuilder cp_;
    operations_research::sat::SatParameters params_;
    operations_research::sat::CpSolverResponse response_;
    bool solved_;
    double solve_time_;

    std::vector<operations_research::sat::IntVar> intVars_;
    struct IntervalComponents {
        operations_research::sat::IntVar start;
        operations_research::sat::IntVar end;
        operations_research::sat::IntVar size;
        operations_research::sat::IntervalVar interval;
    };
    std::vector<IntervalComponents> intervalVars_;

    struct OptionalIntervalComponents {
        operations_research::sat::IntVar start;
        operations_research::sat::IntVar end;
        operations_research::sat::IntVar size;
        operations_research::sat::IntervalVar interval;
        IntVar presence;
    };
    std::vector<OptionalIntervalComponents> optionalIntervalVars_;

    operations_research::sat::IntVar getOrtVar(IntVar var) const {
        return intVars_.at(var.id());
    }

    operations_research::sat::BoolVar getOrtBoolVar(IntVar var) const {
        return intVars_.at(var.id()).ToBoolVar();
    }

    operations_research::sat::IntervalVar getOrtIntervalVar(IntervalVar interval) const {
        return intervalVars_.at(interval.id()).interval;
    }

    operations_research::sat::IntervalVar getOrtOptionalIntervalVar(OptionalIntervalVar interval) const {
        return optionalIntervalVars_.at(interval.id()).interval;
    }

    int addTrackedVar(const operations_research::sat::IntVar& ortVar) {
        int id = static_cast<int>(intVars_.size());
        intVars_.push_back(ortVar);
        return id;
    }

    operations_research::sat::LinearExpr toOrtLinearExpr(const LinearExpr& expr) const {
        operations_research::sat::LinearExpr result;
        for (const auto& [var, coeff] : expr.terms()) {
            result += operations_research::sat::LinearExpr(getOrtVar(var)) * coeff;
        }
        result += expr.constant();
        return result;
    }
};

#else // !ORTOOLS_FOUND

/**
 * @brief Stub implementation when OR-Tools is not available
 */
class ORToolsCPSATBackend : public ICPBackend {
public:
    ORToolsCPSATBackend() {
        throw std::runtime_error("OR-Tools CP-SAT backend is not available. Install OR-Tools with: brew install or-tools");
    }

    std::string name() const override { return "ortools-cpsat"; }

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
    bool exportModel(const std::string&) const override { return false; }
};

#endif // ORTOOLS_FOUND

} // namespace cp
} // namespace routing
