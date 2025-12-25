// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include <set>
#include <string>
#include <vector>

namespace routing {

    // Forward declarations
    class Problem;
    class Tour;
    class Entity;

    // Backward compatibility aliases
    using ComposableProblem = Problem;
    using ComposableTour = Tour;
    using ComposableEntity = Entity;

    /**
     * @brief Context for insertion operations
     *
     * Contains all information needed to evaluate and perform
     * an insertion of a client into a tour.
     */
    struct InsertionContext {
        Entity* client;              // Client to insert
        int position;                // Position in route (0 = after depot)
        Entity* predecessor;         // Node before insertion point (may be depot)
        Entity* successor;           // Node after insertion point (may be depot)
    };

    /**
     * @brief Interface for incremental solution evaluators
     *
     * Evaluators are responsible for checking feasibility and computing
     * cost deltas for move operations in metaheuristic search.
     *
     * Each evaluator is associated with specific attributes (e.g., capacity
     * evaluator requires Consumer and Stock attributes). The PluginRegistry
     * uses this information to activate evaluators based on enabled attributes.
     */
    class IEvaluator {
    public:
        virtual ~IEvaluator() = default;

        /// Returns a unique name for this evaluator
        virtual std::string name() const = 0;

        /// Returns the attribute types required for this evaluator
        virtual std::vector<AttributeTypeId> requiredAttributes() const = 0;

        /// Priority for evaluation ordering (lower = earlier). Default: 100
        virtual int priority() const { return 100; }

        /**
         * @brief Check if an insertion is feasible
         *
         * @param tour The tour to insert into
         * @param ctx The insertion context
         * @return true if the insertion maintains feasibility for this evaluator's constraints
         */
        virtual bool checkFeasibility(const Tour& tour,
                                      const InsertionContext& ctx) const = 0;

        /**
         * @brief Compute the cost delta for an insertion
         *
         * @param tour The tour to insert into
         * @param ctx The insertion context
         * @return The change in cost (positive = worse, negative = better)
         */
        virtual double evaluateInsertionDelta(const Tour& tour,
                                              const InsertionContext& ctx) const = 0;

        /**
         * @brief Apply an insertion and update internal state
         *
         * Called after an insertion is accepted. The evaluator should
         * update any cached state (e.g., total load, arrival times).
         *
         * @param tour The tour being modified
         * @param ctx The insertion context
         */
        virtual void applyInsertion(Tour& tour,
                                    const InsertionContext& ctx) = 0;

        /**
         * @brief Check if this evaluator is applicable given enabled attributes
         */
        bool isApplicable(const std::set<AttributeTypeId>& enabledAttrs) const {
            for (const auto& reqAttr : requiredAttributes()) {
                if (enabledAttrs.find(reqAttr) == enabledAttrs.end()) {
                    return false;
                }
            }
            return true;
        }
    };

} // namespace routing
