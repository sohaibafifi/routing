// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "attributes.hpp"
#include <vector>
#include <set>
#include <string>

#ifdef CPLEX_FOUND
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ilcplex/ilocplexi.h>
#pragma GCC diagnostic pop
#endif

namespace routing {

    // Forward declarations
    class ComposableProblem;

    /**
     * @brief Interface for constraint generators
     *
     * Constraint generators are responsible for adding CPLEX variables,
     * constraints, and objective terms for a specific problem aspect
     * (e.g., capacity, time windows, synchronization).
     *
     * Each generator declares which attributes it requires. The AttributeRegistry
     * uses this to automatically activate generators when their required
     * attributes are enabled on a problem.
     */
    class IConstraintGenerator {
    public:
        virtual ~IConstraintGenerator() = default;

        /// Returns a unique name for this generator (for debugging/logging)
        virtual std::string name() const = 0;

        /// Returns the attribute types that must be enabled for this generator to be active
        virtual std::vector<AttributeTypeId> requiredAttributes() const = 0;

        /// Returns attribute types that conflict with this generator (optional)
        virtual std::vector<AttributeTypeId> conflictingAttributes() const { return {}; }

        /// Priority for ordering generators (lower = earlier). Default: 100
        /// Routing constraints should be ~10, capacity ~50, time windows ~60
        virtual int priority() const { return 100; }

#ifdef CPLEX_FOUND
        /// Add CPLEX decision variables for this constraint type
        virtual void addVariables(ComposableProblem& problem) = 0;

        /// Add CPLEX constraints
        virtual void addConstraints(ComposableProblem& problem) = 0;

        /// Optional: contribute terms to the objective function
        virtual void addObjectiveTerms(ComposableProblem& problem, IloExpr& objectiveExpr) {}
#endif

        /// Check if this generator should be active given the enabled attributes
        bool isApplicable(const std::set<AttributeTypeId>& enabledAttrs) const {
            // Check all required attributes are present
            for (const auto& reqAttr : requiredAttributes()) {
                if (enabledAttrs.find(reqAttr) == enabledAttrs.end()) {
                    return false;
                }
            }
            // Check no conflicting attributes are present
            for (const auto& conflictAttr : conflictingAttributes()) {
                if (enabledAttrs.find(conflictAttr) != enabledAttrs.end()) {
                    return false;
                }
            }
            return true;
        }
    };

} // namespace routing
