// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IIncrementalEvaluator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"

#include <cmath>
#include <limits>
#include <typeindex>
#include <vector>

namespace routing {
namespace evaluators {

/**
 * @brief O(1) incremental capacity evaluator using cached load sums
 *
 * This evaluator replaces CapacityEvaluator with an incremental version
 * that uses cached total load for constant-time feasibility checking.
 *
 * ## Cache Contents
 *
 * - prefixLoad[i]: Cumulative load up to and including position i
 * - totalLoad: Sum of all demands in the route
 * - vehicleCapacity: Vehicle capacity for this route
 *
 * ## Complexity
 *
 * - buildCache(): O(n) - single pass over route
 * - checkFeasibilityIncremental(): O(1) - constant time comparison
 * - evaluateTwoOptIncremental(): O(1) - reversal doesn't change load
 */
class CapacityIncrementalEvaluator : public IIncrementalEvaluator {
public:
    std::string name() const override { return "CapacityIncrementalEvaluator"; }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return {
            std::type_index(typeid(attributes::Consumer)),
            std::type_index(typeid(attributes::Stock))
        };
    }

    int priority() const override { return 50; }

    // ========== Cache Building (O(n)) ==========

    void buildCache(const Tour& tour, RouteCache& cache) const override {
        auto* problem = tour.getProblem();
        if (!problem) return;

        // Get vehicle capacity
        double capacity = std::numeric_limits<double>::infinity();
        auto vehicles = problem->getVehicles();
        Vehicle* vehicle = nullptr;
        for (auto* v : vehicles) {
            if (v && v->getID() == tour.getID()) {
                vehicle = v;
                break;
            }
        }
        if (!vehicle && !vehicles.empty()) {
            vehicle = vehicles.front();
        }
        if (vehicle) {
            if (auto* stock = vehicle->tryGetAttribute<attributes::Stock>()) {
                capacity = stock->getCapacity();
            }
        }
        cache.vehicleCapacity = capacity;

        // Build prefix sums
        const size_t n = tour.getNbClient();
        cache.prefixLoad.resize(n);

        double cumLoad = 0.0;
        for (size_t i = 0; i < n; ++i) {
            auto* client = dynamic_cast<Entity*>(tour.getClient(i));
            if (client) {
                if (auto* consumer = client->tryGetAttribute<attributes::Consumer>()) {
                    cumLoad += consumer->getDemand();
                }
            }
            cache.prefixLoad[i] = cumLoad;
        }
        cache.totalLoad = cumLoad;
    }

    // ========== O(1) Incremental Feasibility ==========

    bool checkFeasibilityIncremental(
        const Tour& /*tour*/,
        const RouteCache& cache,
        const InsertionContext& ctx) const override {

        if (!std::isfinite(cache.vehicleCapacity)) {
            return true;
        }

        double addedDemand = 0.0;
        if (ctx.client) {
            if (auto* consumer = ctx.client->tryGetAttribute<attributes::Consumer>()) {
                addedDemand = consumer->getDemand();
            }
        }

        // O(1): Just check total load + new demand <= capacity
        return (cache.totalLoad + addedDemand) <= cache.vehicleCapacity + 1e-9;
    }

    MoveDelta evaluateInsertionIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const InsertionContext& ctx) const override {

        bool feasible = checkFeasibilityIncremental(tour, cache, ctx);
        // Capacity evaluator contributes 0 to cost (constraints only)
        return MoveDelta(0.0, feasible);
    }

    MoveDelta evaluateRemovalIncremental(
        const Tour& /*tour*/,
        const RouteCache& /*cache*/,
        const RemovalContext& /*ctx*/) const override {
        // Removal always maintains or improves capacity feasibility
        return MoveDelta(0.0, true);
    }

    MoveDelta evaluateTwoOptIncremental(
        const Tour& /*tour*/,
        const RouteCache& /*cache*/,
        const TwoOptContext& /*ctx*/) const override {
        // 2-opt doesn't change total load
        return MoveDelta(0.0, true);
    }

    // ========== Cache Updates ==========

    void updateCacheAfterInsertion(
        const Tour& tour,
        RouteCache& cache,
        const InsertionContext& ctx) const override {

        double addedDemand = 0.0;
        if (ctx.client) {
            if (auto* consumer = ctx.client->tryGetAttribute<attributes::Consumer>()) {
                addedDemand = consumer->getDemand();
            }
        }

        // Update prefix sums after insertion point
        const size_t n = tour.getNbClient();
        cache.prefixLoad.resize(n);

        // Compute prefix at insertion position
        double prefixBefore = (ctx.position > 0) ? cache.prefixLoad[ctx.position - 1] : 0.0;

        // Insert new prefix value
        if (ctx.position < static_cast<int>(n)) {
            cache.prefixLoad.insert(
                cache.prefixLoad.begin() + ctx.position,
                prefixBefore + addedDemand
            );
        }

        // Update all positions after insertion
        for (size_t i = ctx.position + 1; i < cache.prefixLoad.size(); ++i) {
            cache.prefixLoad[i] += addedDemand;
        }

        cache.totalLoad += addedDemand;
    }

    void updateCacheAfterRemoval(
        const Tour& /*tour*/,
        RouteCache& cache,
        const RemovalContext& ctx) const override {

        double removedDemand = 0.0;
        if (ctx.client) {
            if (auto* consumer = ctx.client->tryGetAttribute<attributes::Consumer>()) {
                removedDemand = consumer->getDemand();
            }
        }

        // Update prefix sums after removal point
        for (size_t i = ctx.position; i < cache.prefixLoad.size(); ++i) {
            cache.prefixLoad[i] -= removedDemand;
        }

        // Remove the position from prefix array
        if (ctx.position < static_cast<int>(cache.prefixLoad.size())) {
            cache.prefixLoad.erase(cache.prefixLoad.begin() + ctx.position);
        }

        cache.totalLoad -= removedDemand;
    }

    // ========== Legacy Interface ==========

    bool checkFeasibility(const Tour& tour,
                          const InsertionContext& ctx) const override {
        // Build temporary cache and use incremental method
        RouteCache tempCache;
        buildCache(tour, tempCache);
        return checkFeasibilityIncremental(tour, tempCache, ctx);
    }

    double evaluateInsertionDelta(const Tour& /*tour*/,
                                  const InsertionContext& /*ctx*/) const override {
        return 0.0;
    }

    void applyInsertion(Tour& /*tour*/,
                        const InsertionContext& /*ctx*/) override {}
};

} // namespace evaluators
} // namespace routing