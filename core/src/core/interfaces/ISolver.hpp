// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <string>
#include <functional>

namespace routing {

// Forward declarations
class Problem;
class Configuration;
class Solution;

/// Callback type for solution improvement notifications
/// Parameters: solution pointer, objective value
using ImprovementCallback = std::function<void(Solution*, double)>;

/**
 * @brief Interface for all solver implementations
 */
class ISolver {
public:
    virtual ~ISolver() = default;

    /// Get solver name
    virtual std::string name() const = 0;

    /// Get solver description
    virtual std::string description() const { return ""; }

    /// Set the problem to solve
    virtual void setProblem(Problem* problem) = 0;

    /// Get the current problem
    virtual Problem* getProblem() const = 0;

    /// Set solver configuration
    virtual void setConfiguration(Configuration* config) = 0;

    /// Set default configuration
    virtual void setDefaultConfiguration() = 0;

    /// Solve the problem
    virtual bool solve(double timeout = 3600) = 0;

    /// Get the best solution found
    virtual Solution* getSolution() const = 0;

    /// Get the objective value of the best solution
    virtual double getObjectiveValue() const = 0;

    /// Check if the problem was solved to optimality
    virtual bool isOptimal() const { return false; }

    /// Get solving statistics
    virtual std::string getStats() const { return ""; }

    /// Set callback for solution improvements
    virtual void setImprovementCallback(ImprovementCallback /*callback*/) {}
};

} // namespace routing
