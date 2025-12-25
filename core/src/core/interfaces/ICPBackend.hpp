// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/cp/CPTypes.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <limits>

namespace routing {
namespace cp {

/**
 * @brief Abstract interface for Constraint Programming backends
 *
 * This interface provides a unified API for different CP solvers:
 * - IBM ILOG CP Optimizer
 * - Google OR-Tools CP-SAT
 * - Gecode
 *
 * The interface focuses on the constraints commonly used in vehicle routing:
 * - Interval variables for time windows
 * - Circuit/path constraints
 * - Cumulative constraints for capacity
 * - No-overlap constraints for synchronization
 */
class ICPBackend {
public:
    virtual ~ICPBackend() = default;

    /// Get backend name (e.g., "cpoptimizer", "ortools", "gecode")
    virtual std::string name() const = 0;

    // ========== Variable Creation ==========

    /**
     * @brief Create an integer variable
     * @param lb Lower bound
     * @param ub Upper bound
     * @param name Optional variable name
     */
    virtual IntVar newIntVar(int lb, int ub, const std::string& name = "") = 0;

    /**
     * @brief Create a boolean variable (0-1)
     * @param name Optional variable name
     */
    virtual IntVar newBoolVar(const std::string& name = "") = 0;

    /**
     * @brief Create an integer constant
     * @param value Constant value
     */
    virtual IntVar newConstant(int value) = 0;

    /**
     * @brief Create an interval variable for scheduling
     * @param minStart Earliest possible start time
     * @param maxEnd Latest possible end time
     * @param minDuration Minimum duration
     * @param maxDuration Maximum duration
     * @param name Optional variable name
     */
    virtual IntervalVar newIntervalVar(int minStart, int maxEnd,
                                       int minDuration, int maxDuration,
                                       const std::string& name = "") = 0;

    /**
     * @brief Create an interval variable with fixed duration
     */
    virtual IntervalVar newFixedIntervalVar(int minStart, int maxEnd,
                                           int duration,
                                           const std::string& name = "") {
        return newIntervalVar(minStart, maxEnd, duration, duration, name);
    }

    /**
     * @brief Create an optional interval variable
     * @param minStart Earliest possible start time
     * @param maxEnd Latest possible end time
     * @param minDuration Minimum duration
     * @param maxDuration Maximum duration
     * @param name Optional variable name
     * @return OptionalIntervalVar with presence indicator
     */
    virtual OptionalIntervalVar newOptionalIntervalVar(int minStart, int maxEnd,
                                                       int minDuration, int maxDuration,
                                                       const std::string& name = "") = 0;

    // ========== Basic Constraints ==========

    /**
     * @brief Add constraint: expr == value
     */
    virtual void addEquality(const LinearExpr& expr, int value) = 0;

    /**
     * @brief Add constraint: var1 == var2
     */
    virtual void addEquality(IntVar var1, IntVar var2) = 0;

    /**
     * @brief Add constraint: var == value
     */
    virtual void addEquality(IntVar var, int value) {
        addEquality(LinearExpr(var), value);
    }

    /**
     * @brief Add constraint: lb <= expr <= ub
     */
    virtual void addLinearConstraint(const LinearExpr& expr, int lb, int ub) = 0;

    /**
     * @brief Add constraint: expr <= ub
     */
    virtual void addLessOrEqual(const LinearExpr& expr, int ub) {
        addLinearConstraint(expr, std::numeric_limits<int>::min(), ub);
    }

    /**
     * @brief Add constraint: expr >= lb
     */
    virtual void addGreaterOrEqual(const LinearExpr& expr, int lb) {
        addLinearConstraint(expr, lb, std::numeric_limits<int>::max());
    }

    /**
     * @brief Add constraint: var1 <= var2 + offset
     */
    virtual void addLessOrEqual(IntVar var1, IntVar var2, int offset = 0) = 0;

    /**
     * @brief Add implication: if condition is true, then expr op bound
     */
    virtual void addImplication(IntVar condition, const LinearExpr& expr, int lb, int ub) = 0;

    // ========== Global Constraints ==========

    /**
     * @brief All-different constraint
     * All variables must take different values
     */
    virtual void addAllDifferent(const std::vector<IntVar>& vars) = 0;

    /**
     * @brief Element constraint: result = array[index]
     */
    virtual void addElement(IntVar index, const std::vector<int>& array, IntVar result) = 0;

    /**
     * @brief Element constraint with variable array: result = vars[index]
     */
    virtual void addElement(IntVar index, const std::vector<IntVar>& vars, IntVar result) = 0;

    /**
     * @brief Circuit constraint
     * Variables define a Hamiltonian circuit over nodes 0..n-1
     * next[i] = j means arc from i to j
     */
    virtual void addCircuit(const std::vector<IntVar>& next) = 0;

    /**
     * @brief Sub-circuit constraint (allows nodes to point to themselves)
     * Used for TSP with optional nodes
     */
    virtual void addSubCircuit(const std::vector<IntVar>& next) = 0;

    /**
     * @brief Inverse constraint: next[prev[i]] == i and prev[next[i]] == i
     */
    virtual void addInverse(const std::vector<IntVar>& next,
                           const std::vector<IntVar>& prev) = 0;

    // ========== Scheduling Constraints ==========

    /**
     * @brief No-overlap constraint
     * Intervals cannot overlap in time
     */
    virtual void addNoOverlap(const std::vector<IntervalVar>& intervals) = 0;

    /**
     * @brief No-overlap constraint with optional intervals
     */
    virtual void addNoOverlap(const std::vector<OptionalIntervalVar>& intervals) = 0;

    /**
     * @brief Cumulative constraint
     * At any time point, sum of demands of active intervals <= capacity
     */
    virtual void addCumulative(const std::vector<IntervalVar>& intervals,
                               const std::vector<int>& demands,
                               int capacity) = 0;

    /**
     * @brief Cumulative with optional intervals
     */
    virtual void addCumulative(const std::vector<OptionalIntervalVar>& intervals,
                               const std::vector<int>& demands,
                               int capacity) = 0;

    /**
     * @brief Precedence constraint: interval1 ends before interval2 starts
     * end(interval1) + delay <= start(interval2)
     */
    virtual void addEndBeforeStart(IntervalVar interval1, IntervalVar interval2,
                                   int delay = 0) = 0;

    /**
     * @brief Get start variable of an interval
     */
    virtual IntVar startOf(IntervalVar interval) = 0;

    /**
     * @brief Get end variable of an interval
     */
    virtual IntVar endOf(IntervalVar interval) = 0;

    /**
     * @brief Get duration variable of an interval
     */
    virtual IntVar durationOf(IntervalVar interval) = 0;

    /**
     * @brief Get start variable of optional interval (undefined if absent)
     */
    virtual IntVar startOf(OptionalIntervalVar interval, int defaultValue = 0) = 0;

    /**
     * @brief Get end variable of optional interval (undefined if absent)
     */
    virtual IntVar endOf(OptionalIntervalVar interval, int defaultValue = 0) = 0;

    // ========== Objective ==========

    /**
     * @brief Set objective to minimize
     */
    virtual void minimize(const LinearExpr& expr) = 0;

    /**
     * @brief Set objective to maximize
     */
    virtual void maximize(const LinearExpr& expr) = 0;

    /**
     * @brief Minimize the makespan (max end time of intervals)
     */
    virtual void minimizeMakespan(const std::vector<IntervalVar>& intervals) = 0;

    // ========== Solving ==========

    /**
     * @brief Solve the model
     * @param timeout Maximum solving time in seconds
     * @return Solver status
     */
    virtual CPStatus solve(double timeout = 3600.0) = 0;

    /**
     * @brief Provide hints/warm start
     */
    virtual void setHint(const std::vector<std::pair<IntVar, int>>& hints) = 0;

    /**
     * @brief Set number of worker threads
     */
    virtual void setNumWorkers(int workers) = 0;

    /**
     * @brief Set random seed for reproducibility
     */
    virtual void setRandomSeed(int seed) = 0;

    // ========== Solution Access ==========

    /**
     * @brief Get value of integer variable in solution
     */
    virtual int getValue(IntVar var) const = 0;

    /**
     * @brief Get value of boolean variable (0 or 1)
     */
    virtual bool getBoolValue(IntVar var) const = 0;

    /**
     * @brief Get start time of interval in solution
     */
    virtual int getStart(IntervalVar interval) const = 0;

    /**
     * @brief Get end time of interval in solution
     */
    virtual int getEnd(IntervalVar interval) const = 0;

    /**
     * @brief Get duration of interval in solution
     */
    virtual int getDuration(IntervalVar interval) const = 0;

    /**
     * @brief Check if optional interval is present in solution
     */
    virtual bool isPresent(OptionalIntervalVar interval) const = 0;

    /**
     * @brief Get objective value of solution
     */
    virtual double getObjectiveValue() const = 0;

    /**
     * @brief Get lower bound on objective
     */
    virtual double getObjectiveBound() const = 0;

    /**
     * @brief Get gap between objective and bound
     */
    virtual double getGap() const {
        double obj = getObjectiveValue();
        double bound = getObjectiveBound();
        if (std::abs(obj) < 1e-10) return 0.0;
        return std::abs(obj - bound) / std::abs(obj);
    }

    // ========== Statistics ==========

    /**
     * @brief Get solving time in seconds
     */
    virtual double getSolveTime() const = 0;

    /**
     * @brief Get number of search branches
     */
    virtual long long getNumBranches() const = 0;

    /**
     * @brief Get number of failures/backtracks
     */
    virtual long long getNumFailures() const = 0;

    // ========== Model Management ==========

    /**
     * @brief Clear the model (remove all variables and constraints)
     */
    virtual void clear() = 0;

    /**
     * @brief Export model to file (format depends on backend)
     */
    virtual bool exportModel(const std::string& filename) const { return false; }
};

/**
 * @brief Factory function type for creating CP backends
 */
using CPBackendFactory = std::function<std::unique_ptr<ICPBackend>()>;

} // namespace cp
} // namespace routing
