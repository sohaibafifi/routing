// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/mip/MIPTypes.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <limits>

namespace routing {
namespace mip {

/**
 * @brief Abstract interface for Mixed Integer Programming backends
 *
 * This interface provides a unified API for different MIP solvers:
 * - IBM ILOG CPLEX
 * - Gurobi
 * - HiGHS
 * - SCIP
 * - CBC
 *
 * The interface focuses on building and solving MIP models for vehicle routing.
 */
class IMIPBackend {
public:
    virtual ~IMIPBackend() = default;

    /// Get backend name (e.g., "cplex", "gurobi", "highs")
    virtual std::string name() const = 0;

    // ========== Variable Creation ==========

    /**
     * @brief Create a continuous variable
     * @param lb Lower bound (default: 0)
     * @param ub Upper bound (default: infinity)
     * @param name Optional variable name
     */
    virtual Var newVar(double lb = 0.0, double ub = std::numeric_limits<double>::infinity(),
                       const std::string& name = "") = 0;

    /**
     * @brief Create an integer variable
     * @param lb Lower bound
     * @param ub Upper bound
     * @param name Optional variable name
     */
    virtual IntVar newIntVar(int lb, int ub, const std::string& name = "") = 0;

    /**
     * @brief Create a binary variable (0-1)
     * @param name Optional variable name
     */
    virtual BoolVar newBoolVar(const std::string& name = "") = 0;

    /**
     * @brief Create a 2D array of binary variables
     * @param rows Number of rows
     * @param cols Number of columns
     * @param namePrefix Prefix for variable names
     */
    virtual std::vector<std::vector<BoolVar>> newBoolVarArray2D(
        size_t rows, size_t cols, const std::string& namePrefix = "") {
        std::vector<std::vector<BoolVar>> vars(rows);
        for (size_t i = 0; i < rows; ++i) {
            vars[i].resize(cols);
            for (size_t j = 0; j < cols; ++j) {
                std::string varName = namePrefix.empty() ? "" :
                    namePrefix + "_" + std::to_string(i) + "_" + std::to_string(j);
                vars[i][j] = newBoolVar(varName);
            }
        }
        return vars;
    }

    // ========== Constraint Addition ==========

    /**
     * @brief Add constraint: expr <= rhs
     */
    virtual void addLessEqual(const LinearExpr& expr, double rhs,
                              const std::string& name = "") = 0;

    /**
     * @brief Add constraint: expr >= rhs
     */
    virtual void addGreaterEqual(const LinearExpr& expr, double rhs,
                                 const std::string& name = "") = 0;

    /**
     * @brief Add constraint: expr == rhs
     */
    virtual void addEqual(const LinearExpr& expr, double rhs,
                          const std::string& name = "") = 0;

    /**
     * @brief Add constraint: lb <= expr <= ub
     */
    virtual void addRange(const LinearExpr& expr, double lb, double ub,
                          const std::string& name = "") = 0;

    /**
     * @brief Add constraint: expr1 == expr2
     */
    virtual void addEqual(const LinearExpr& expr1, const LinearExpr& expr2,
                          const std::string& name = "") {
        // Default implementation: expr1 - expr2 == 0
        LinearExpr diff = expr1;
        for (const auto& [varId, coeff] : expr2.terms()) {
            diff.addTerm(Var(varId), -coeff);
        }
        diff.addConstant(-expr2.constant());
        addEqual(diff, 0.0, name);
    }

    /**
     * @brief Add indicator constraint: if indicator == value, then expr sense rhs
     * @param indicator Boolean variable
     * @param value Value that activates the constraint (0 or 1)
     * @param expr Linear expression
     * @param sense Constraint sense
     * @param rhs Right-hand side
     */
    virtual void addIndicator(BoolVar indicator, int value,
                              const LinearExpr& expr, Sense sense, double rhs,
                              const std::string& name = "") = 0;

    /**
     * @brief Add SOS1 constraint (at most one variable in set is non-zero)
     */
    virtual void addSOS1(const std::vector<Var>& vars,
                         const std::vector<double>& weights = {},
                         const std::string& name = "") = 0;

    /**
     * @brief Add SOS2 constraint (at most two adjacent variables are non-zero)
     */
    virtual void addSOS2(const std::vector<Var>& vars,
                         const std::vector<double>& weights = {},
                         const std::string& name = "") = 0;

    // ========== Lazy Constraints and User Cuts ==========

    /**
     * @brief Add a lazy constraint (checked only when integer solution found)
     */
    virtual void addLazyConstraint(const LinearExpr& expr, Sense sense, double rhs) = 0;

    /**
     * @brief Add a user cut (can be added at any time, must not cut off feasible solutions)
     */
    virtual void addUserCut(const LinearExpr& expr, Sense sense, double rhs) = 0;

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
     * @brief Set quadratic objective to minimize
     */
    virtual void minimizeQuad(const QuadExpr& expr) = 0;

    /**
     * @brief Set quadratic objective to maximize
     */
    virtual void maximizeQuad(const QuadExpr& expr) = 0;

    // ========== Warm Starting ==========

    /**
     * @brief Provide a warm start solution
     * @param vars Variables to set
     * @param values Values for the variables
     */
    virtual void setWarmStart(const std::vector<Var>& vars,
                              const std::vector<double>& values) = 0;

    /**
     * @brief Provide MIP start hint for integer variables
     */
    virtual void setMIPStart(const std::vector<std::pair<Var, double>>& solution) = 0;

    // ========== Solving ==========

    /**
     * @brief Solve the model
     * @param timeout Maximum solving time in seconds
     * @return Solver status
     */
    virtual MIPStatus solve(double timeout = 3600.0) = 0;

    // ========== Solution Access ==========

    /**
     * @brief Get value of variable in solution
     */
    virtual double getValue(Var var) const = 0;

    /**
     * @brief Get value of integer variable in solution
     */
    virtual int getIntValue(IntVar var) const {
        return static_cast<int>(std::round(getValue(Var(var.id()))));
    }

    /**
     * @brief Get value of boolean variable (0 or 1)
     */
    virtual bool getBoolValue(BoolVar var) const {
        return getValue(Var(var.id())) > 0.5;
    }

    /**
     * @brief Get objective value of solution
     */
    virtual double getObjectiveValue() const = 0;

    /**
     * @brief Get best objective bound
     */
    virtual double getObjectiveBound() const = 0;

    /**
     * @brief Get MIP gap
     */
    virtual double getGap() const {
        double obj = getObjectiveValue();
        double bound = getObjectiveBound();
        if (std::abs(obj) < 1e-10) return 0.0;
        return std::abs(obj - bound) / std::abs(obj);
    }

    /**
     * @brief Get reduced cost of a variable
     */
    virtual double getReducedCost(Var var) const = 0;

    /**
     * @brief Get dual value (shadow price) of a constraint
     * @param constraintIndex Index of the constraint
     */
    virtual double getDual(int constraintIndex) const = 0;

    // ========== Statistics ==========

    /**
     * @brief Get solving time in seconds
     */
    virtual double getSolveTime() const = 0;

    /**
     * @brief Get number of simplex iterations
     */
    virtual long long getNumIterations() const = 0;

    /**
     * @brief Get number of nodes explored in branch-and-bound
     */
    virtual long long getNumNodes() const = 0;

    /**
     * @brief Get number of solutions found
     */
    virtual int getNumSolutions() const = 0;

    // ========== Parameters ==========

    /**
     * @brief Set time limit in seconds
     */
    virtual void setTimeLimit(double seconds) = 0;

    /**
     * @brief Set MIP gap tolerance
     */
    virtual void setGapTolerance(double gap) = 0;

    /**
     * @brief Set number of threads
     */
    virtual void setNumThreads(int threads) = 0;

    /**
     * @brief Set random seed
     */
    virtual void setRandomSeed(int seed) = 0;

    /**
     * @brief Set verbosity level (0 = quiet, higher = more verbose)
     */
    virtual void setVerbosity(int level) = 0;

    /**
     * @brief Enable/disable presolve
     */
    virtual void setPresolve(bool enable) = 0;

    /**
     * @brief Set node limit
     */
    virtual void setNodeLimit(long long limit) = 0;

    /**
     * @brief Set solution limit
     */
    virtual void setSolutionLimit(int limit) = 0;

    // ========== Model Management ==========

    /**
     * @brief Clear the model (remove all variables and constraints)
     */
    virtual void clear() = 0;

    /**
     * @brief Get number of variables
     */
    virtual int getNumVars() const = 0;

    /**
     * @brief Get number of constraints
     */
    virtual int getNumConstraints() const = 0;

    /**
     * @brief Get number of non-zeros in constraint matrix
     */
    virtual long long getNumNonZeros() const = 0;

    /**
     * @brief Export model to file
     * @param filename File path (extension determines format: .lp, .mps, .sav)
     */
    virtual bool exportModel(const std::string& filename) const = 0;

    /**
     * @brief Import model from file
     */
    virtual bool importModel(const std::string& filename) = 0;

    // ========== Callbacks (Optional Advanced Feature) ==========

    /**
     * @brief Callback function type for lazy constraints
     * Called when an integer feasible solution is found.
     * The callback receives the backend to query solution values and add cuts.
     */
    using LazyCallback = std::function<void(IMIPBackend& backend)>;

    /**
     * @brief Callback function type for user cuts
     * Called during the branch-and-bound process.
     */
    using CutCallback = std::function<void(IMIPBackend& backend)>;

    /**
     * @brief Callback for incumbent solutions
     * Called when a new best solution is found.
     */
    using IncumbentCallback = std::function<void(IMIPBackend& backend, double objValue)>;

    /**
     * @brief Set lazy constraint callback
     */
    virtual void setLazyCallback(LazyCallback callback) {
        lazyCallback_ = std::move(callback);
    }

    /**
     * @brief Set user cut callback
     */
    virtual void setCutCallback(CutCallback callback) {
        cutCallback_ = std::move(callback);
    }

    /**
     * @brief Set incumbent callback
     */
    virtual void setIncumbentCallback(IncumbentCallback callback) {
        incumbentCallback_ = std::move(callback);
    }

    // ========== Advanced: Access to Relaxation ==========

    /**
     * @brief Solve LP relaxation only
     */
    virtual MIPStatus solveRelaxation() = 0;

    /**
     * @brief Fix integer variables to their current values and resolve
     * Useful for getting dual values after MIP solve
     */
    virtual MIPStatus solveFixed() = 0;

protected:
    LazyCallback lazyCallback_;
    CutCallback cutCallback_;
    IncumbentCallback incumbentCallback_;
};

/**
 * @brief Factory function type for creating MIP backends
 */
using MIPBackendFactory = std::function<std::unique_ptr<IMIPBackend>()>;

} // namespace mip
} // namespace routing
