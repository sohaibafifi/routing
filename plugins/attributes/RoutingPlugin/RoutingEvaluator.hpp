// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IIncrementalEvaluator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"

#include <typeindex>
#include <vector>

namespace routing {
namespace evaluators {

/**
 * @brief Routing evaluator with incremental support
 *
 * This evaluator computes distance-based costs for insertions.
 * It's already O(1) by design, but now extends IIncrementalEvaluator
 * for consistent interface and cache-based operation support.
 *
 * ## Cache Contents
 *
 * - prefixDistance[i]: Cumulative distance up to position i
 * - totalDistance: Total route distance
 *
 * ## Complexity
 *
 * All operations are O(1):
 * - Insertion delta: (dist[pred→client] + dist[client→succ]) - dist[pred→succ]
 * - 2-opt delta: simple edge swap calculation
 */
class RoutingEvaluator : public IIncrementalEvaluator {
public:
    std::string name() const override { return "RoutingEvaluator"; }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return { std::type_index(typeid(attributes::GeoNode)) };
    }

    int priority() const override { return 10; }

    // ========== Cache Building ==========

    void buildCache(const Tour& tour, RouteCache& cache) const override {
        auto* problem = tour.getProblem();
        if (!problem) return;

        const size_t n = tour.getNbClient();
        cache.prefixDistance.resize(n);

        if (n == 0) {
            cache.totalDistance = 0.0;
            return;
        }

        auto* depot = dynamic_cast<Entity*>(problem->getDepot());
        if (!depot) {
            cache.totalDistance = 0.0;
            return;
        }

        double cumDist = 0.0;
        Entity* prev = depot;

        for (size_t i = 0; i < n; ++i) {
            auto* client = dynamic_cast<Entity*>(tour.getClient(i));
            if (!client) continue;

            cumDist += problem->getDistanceEntity(*prev, *client);
            cache.prefixDistance[i] = cumDist;
            prev = client;
        }

        // Add return to depot
        cumDist += problem->getDistanceEntity(*prev, *depot);
        cache.totalDistance = cumDist;
    }

    // ========== Incremental Evaluation (already O(1)) ==========

    bool checkFeasibilityIncremental(
        const Tour& /*tour*/,
        const RouteCache& /*cache*/,
        const InsertionContext& /*ctx*/) const override {
        // Routing has no feasibility constraints
        return true;
    }

    MoveDelta evaluateInsertionIncremental(
        const Tour& tour,
        const RouteCache& /*cache*/,
        const InsertionContext& ctx) const override {

        double delta = evaluateInsertionDelta(tour, ctx);
        return MoveDelta(delta, true);
    }

    MoveDelta evaluateRemovalIncremental(
        const Tour& tour,
        const RouteCache& /*cache*/,
        const RemovalContext& ctx) const override {

        auto* problem = tour.getProblem();
        if (!problem || !ctx.client || !ctx.predecessor || !ctx.successor) {
            return MoveDelta(0.0, true);
        }

        // Cost saved by removing client
        const double via = problem->getDistanceEntity(*ctx.predecessor, *ctx.client)
                         + problem->getDistanceEntity(*ctx.client, *ctx.successor);
        const double direct = problem->getDistanceEntity(*ctx.predecessor, *ctx.successor);

        // Delta is negative (improvement) when removing
        return MoveDelta(direct - via, true);
    }

    MoveDelta evaluateTwoOptIncremental(
        const Tour& tour,
        const RouteCache& /*cache*/,
        const TwoOptContext& ctx) const override {

        auto* problem = tour.getProblem();
        if (!problem) return MoveDelta(0.0, true);

        const size_t n = tour.getNbClient();
        if (ctx.i < 0 || ctx.j < 0 ||
            ctx.i >= ctx.j || ctx.j >= static_cast<int>(n)) {
            return MoveDelta(0.0, true);
        }

        // 2-opt removes edges (i, i+1) and (j, j+1) and adds (i, j) and (i+1, j+1)
        auto* nodeI = dynamic_cast<Entity*>(tour.getClient(ctx.i));
        auto* nodeI1 = dynamic_cast<Entity*>(tour.getClient(ctx.i + 1));
        auto* nodeJ = dynamic_cast<Entity*>(tour.getClient(ctx.j));

        Entity* nodeJ1 = nullptr;
        auto* depot = dynamic_cast<Entity*>(problem->getDepot());

        if (ctx.j + 1 < static_cast<int>(n)) {
            nodeJ1 = dynamic_cast<Entity*>(tour.getClient(ctx.j + 1));
        } else {
            nodeJ1 = depot;
        }

        if (!nodeI || !nodeI1 || !nodeJ || !nodeJ1) {
            return MoveDelta(0.0, true);
        }

        // Old edges cost
        double oldCost = problem->getDistanceEntity(*nodeI, *nodeI1)
                       + problem->getDistanceEntity(*nodeJ, *nodeJ1);

        // New edges cost
        double newCost = problem->getDistanceEntity(*nodeI, *nodeJ)
                       + problem->getDistanceEntity(*nodeI1, *nodeJ1);

        return MoveDelta(newCost - oldCost, true);
    }

    // ========== Legacy Interface ==========

    bool checkFeasibility(const Tour& /*tour*/,
                          const InsertionContext& /*ctx*/) const override {
        return true;
    }

    double evaluateInsertionDelta(const Tour& tour,
                                  const InsertionContext& ctx) const override {
        auto* problem = tour.getProblem();
        if (!problem || !ctx.client || !ctx.predecessor || !ctx.successor) {
            return 0.0;
        }

        const double direct = problem->getDistanceEntity(*ctx.predecessor, *ctx.successor);
        const double via = problem->getDistanceEntity(*ctx.predecessor, *ctx.client)
                         + problem->getDistanceEntity(*ctx.client, *ctx.successor);
        return via - direct;
    }

    void applyInsertion(Tour& /*tour*/,
                        const InsertionContext& /*ctx*/) override {}
};

} // namespace evaluators
} // namespace routing
