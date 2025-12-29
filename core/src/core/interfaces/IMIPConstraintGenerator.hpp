// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IMIPBackend.hpp"
#include "core/interfaces/IAttribute.hpp"
#include <string>
#include <vector>
#include <typeindex>
#include <set>

namespace routing {

// Forward declaration
class Problem;
class Solution;

namespace mip {

/**
 * @brief Interface for MIP constraint generators
 *
 * Similar to ICPConstraintGenerator but uses IMIPBackend instead.
 * Each generator adds variables, constraints, and objective terms
 * for a specific aspect of the problem (routing, time windows, capacity, etc.)
 */
class IMIPConstraintGenerator {
public:
    virtual ~IMIPConstraintGenerator() = default;

    /// Name of this generator
    virtual std::string name() const = 0;

    /// Priority for ordering (lower = earlier)
    virtual int priority() const { return 100; }

    /// Attribute types required by this generator
    virtual std::vector<AttributeTypeId> requiredAttributes() const = 0;

    /**
     * @brief Add variables to the MIP model
     * @param mip The MIP backend
     * @param problem The problem instance
     */
    virtual void addVariables(IMIPBackend& mip, Problem& problem) = 0;

    /**
     * @brief Add constraints to the MIP model
     * @param mip The MIP backend
     * @param problem The problem instance
     */
    virtual void addConstraints(IMIPBackend& mip, Problem& problem) = 0;

    /**
     * @brief Add terms to the objective function
     * @param mip The MIP backend
     * @param problem The problem instance
     * @param expr The linear expression to add to
     */
    virtual void addObjectiveTerms(IMIPBackend& mip, Problem& problem, LinearExpr& expr) {}

    /**
     * @brief Called after solving to extract solution data
     * @param mip The MIP backend (with solution)
     * @param problem The problem instance
     * @param solution The solution to populate
     */
    virtual void extractSolution(const IMIPBackend& mip, Problem& problem, Solution& solution) {}
};

} // namespace mip
} // namespace routing
