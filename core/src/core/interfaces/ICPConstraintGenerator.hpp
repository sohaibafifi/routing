// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPBackend.hpp"
#include "core/interfaces/IAttribute.hpp"
#include <string>
#include <vector>
#include <typeindex>
#include <set>

namespace routing {

// Forward declaration
class Problem;

namespace cp {

/**
 * @brief Interface for CP constraint generators
 *
 * Similar to IConstraintGenerator but uses ICPBackend instead of CPLEX.
 * Each generator adds variables, constraints, and objective terms
 * for a specific aspect of the problem (routing, time windows, capacity, etc.)
 */
class ICPConstraintGenerator {
public:
    virtual ~ICPConstraintGenerator() = default;

    /// Name of this generator
    virtual std::string name() const = 0;

    /// Priority for ordering (lower = earlier)
    virtual int priority() const { return 100; }

    /// Attribute types required by this generator
    virtual std::vector<AttributeTypeId> requiredAttributes() const = 0;

    /**
     * @brief Add variables to the CP model
     * @param cp The CP backend
     * @param problem The problem instance
     */
    virtual void addVariables(ICPBackend& cp, Problem& problem) = 0;

    /**
     * @brief Add constraints to the CP model
     * @param cp The CP backend
     * @param problem The problem instance
     */
    virtual void addConstraints(ICPBackend& cp, Problem& problem) = 0;

    /**
     * @brief Add terms to the objective function
     * @param cp The CP backend
     * @param problem The problem instance
     * @param expr The linear expression to add to
     */
    virtual void addObjectiveTerms(ICPBackend& cp, Problem& problem, LinearExpr& expr) {}

    /**
     * @brief Called after solving to extract solution data
     * @param cp The CP backend (with solution)
     * @param problem The problem instance
     */
    virtual void extractSolution(const ICPBackend& cp, Problem& problem) {}
};

} // namespace cp
} // namespace routing
