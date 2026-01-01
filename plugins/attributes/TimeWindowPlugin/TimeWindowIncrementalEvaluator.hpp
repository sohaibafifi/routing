// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IIncrementalEvaluator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"

#include <algorithm>
#include <limits>
#include <typeindex>
#include <vector>

namespace routing {
namespace evaluators {

/**
 * @brief O(1) incremental time window evaluator using cached arrival times
 *
 * This evaluator replaces TimeWindowEvaluator with an incremental version
 * that uses cached route state for constant-time feasibility checking.
 *
 * ## Key Insight
 *
 * For insertion at position p:
 * - Positions 0..p-1 are unaffected
 * - Position p is the new node
 * - Positions p+1..n-1 may have delayed arrivals
 *
 * Using maxForwardShift cache:
 * - If the delay caused by insertion <= maxForwardShift[p], route stays feasible
 * - No need to simulate entire route
 *
 * ## Cache Contents
 *
 * - arrivalTime[i]: Earliest arrival time at position i
 * - startTime[i]: Start of service at position i (max of arrival and TW open)
 * - departureTime[i]: Departure from position i (start + service)
 * - waitingTime[i]: Idle time at position i (start - arrival)
 * - maxForwardShift[i]: Max delay at position i without violating downstream TW
 *
 * ## Complexity
 *
 * - buildCache(): O(n) - two passes over route
 * - checkFeasibilityIncremental(): O(1) - constant time lookup
 * - evaluateTwoOptIncremental(): O(j-i) - simulate reversed segment only
 */
class TimeWindowIncrementalEvaluator : public IIncrementalEvaluator {
public:
    std::string name() const override { return "TimeWindowIncrementalEvaluator"; }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return {
            std::type_index(typeid(attributes::Rendezvous)),
            std::type_index(typeid(attributes::ServiceQuery))
        };
    }

    int priority() const override { return 60; }

    // ========== Cache Building (O(n)) ==========

    void buildCache(const Tour& tour, RouteCache& cache) const override {
        auto* problem = tour.getProblem();
        if (!problem) return;

        const size_t n = tour.getNbClient();
        cache.resize(n);

        if (n == 0) return;

        auto* depot = problem->getDepot();
        auto* depotEntity = dynamic_cast<Entity*>(depot);
        if (!depotEntity) return;

        // Get depot time window
        double depotOpen = 0.0;
        double depotClose = std::numeric_limits<double>::max();
        if (auto* tw = depotEntity->tryGetAttribute<attributes::Rendezvous>()) {
            depotOpen = tw->getTwOpen();
            depotClose = tw->getTwClose();
        }
        cache.depotCloseTime = depotClose;

        // Helper to get time window
        auto getWindow = [](Entity* entity) -> std::pair<double, double> {
            double open = 0.0;
            double close = std::numeric_limits<double>::max();
            if (entity) {
                if (auto* tw = entity->tryGetAttribute<attributes::Rendezvous>()) {
                    open = tw->getTwOpen();
                    close = tw->getTwClose();
                }
            }
            return {open, close};
        };

        // Helper to get service time
        auto getService = [](Entity* entity) -> double {
            if (entity) {
                if (auto* sq = entity->tryGetAttribute<attributes::ServiceQuery>()) {
                    return sq->getService();
                }
            }
            return 0.0;
        };

        // ========== Forward Pass ==========
        // Compute arrival, start, departure, waiting times

        Entity* prev = depotEntity;
        double currentTime = depotOpen;

        for (size_t i = 0; i < n; ++i) {
            auto* client = dynamic_cast<Entity*>(tour.getClient(i));
            if (!client) continue;

            double travel = problem->getDistanceEntity(*prev, *client);
            double arrival = currentTime + travel;

            auto [open, close] = getWindow(client);
            double start = std::max(arrival, open);
            double service = getService(client);
            double departure = start + service;

            cache.arrivalTime[i] = arrival;
            cache.startTime[i] = start;
            cache.departureTime[i] = departure;
            cache.waitingTime[i] = start - arrival;

            currentTime = departure;
            prev = client;
        }

        // Compute return time to depot
        cache.returnToDepotTime = currentTime + problem->getDistanceEntity(*prev, *depotEntity);

        // ========== Backward Pass ==========
        // Compute maxForwardShift[i] = max delay at position i without violating downstream

        // Start from depot return
        double slack = depotClose - cache.returnToDepotTime;

        for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
            auto* client = dynamic_cast<Entity*>(tour.getClient(i));
            auto [open, close] = getWindow(client);

            // Local slack: how much can we delay start at this position
            double localSlack = close - cache.startTime[i];

            // Total slack: min of local slack and downstream slack (absorbed by waiting)
            // If we delay departure by D at position i:
            //   - Arrival at i+1 increases by D
            //   - If D <= waiting[i+1], start at i+1 unchanged (absorbed)
            //   - Otherwise, D - waiting[i+1] propagates downstream
            //
            // maxForwardShift[i] = min(localSlack, slack + waiting[i+1])
            // where slack is the accumulated downstream slack

            if (i < static_cast<int>(n) - 1) {
                // Waiting time at next position can absorb delay
                slack = std::min(localSlack, slack + cache.waitingTime[i + 1]);
            } else {
                // Last position: consider return to depot
                slack = std::min(localSlack, slack);
            }

            cache.maxForwardShift[i] = std::max(0.0, slack);
        }
    }

    // ========== O(1) Incremental Feasibility ==========

    bool checkFeasibilityIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const InsertionContext& ctx) const override {

        auto* problem = tour.getProblem();
        if (!problem || !ctx.client) return true;

        const size_t n = tour.getNbClient();
        const int pos = ctx.position;

        // Get new node's time window
        double newOpen = 0.0, newClose = std::numeric_limits<double>::max();
        if (auto* tw = ctx.client->tryGetAttribute<attributes::Rendezvous>()) {
            newOpen = tw->getTwOpen();
            newClose = tw->getTwClose();
        }
        double newService = 0.0;
        if (auto* sq = ctx.client->tryGetAttribute<attributes::ServiceQuery>()) {
            newService = sq->getService();
        }

        // Compute departure time from predecessor
        double departureFromPred;
        if (pos == 0) {
            // Inserting at start: predecessor is depot
            auto* depot = dynamic_cast<Entity*>(problem->getDepot());
            double depotOpen = 0.0;
            if (auto* tw = depot->tryGetAttribute<attributes::Rendezvous>()) {
                depotOpen = tw->getTwOpen();
            }
            departureFromPred = depotOpen;
        } else {
            // Predecessor is the client at position (pos-1)
            departureFromPred = cache.departureTime[pos - 1];
        }

        // Travel time from predecessor to new node
        double travelToPred = problem->getDistanceEntity(ctx.predecessor, ctx.client);
        double arrivalAtNew = departureFromPred + travelToPred;
        double startAtNew = std::max(arrivalAtNew, newOpen);

        // Check: can we start service before the new node's close time?
        if (startAtNew > newClose + 1e-9) {
            return false;
        }

        double departureFromNew = startAtNew + newService;

        // Travel time from new node to successor
        double travelToSucc = problem->getDistanceEntity(ctx.client, ctx.successor);
        double newArrivalAtSucc = departureFromNew + travelToSucc;

        if (pos < static_cast<int>(n)) {
            // There's a successor in the current route
            double oldArrivalAtSucc = cache.arrivalTime[pos];
            double delay = newArrivalAtSucc - oldArrivalAtSucc;

            // If delay is negative or zero, no problem
            if (delay > 1e-9) {
                // Check if delay can be absorbed by downstream slack
                double availableShift = cache.maxForwardShift[pos];
                if (delay > availableShift + 1e-9) {
                    return false;
                }
            }
        } else {
            // Inserting at end, successor is depot
            if (newArrivalAtSucc > cache.depotCloseTime + 1e-9) {
                return false;
            }
        }

        return true;
    }

    MoveDelta evaluateInsertionIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const InsertionContext& ctx) const override {

        bool feasible = checkFeasibilityIncremental(tour, cache, ctx);
        // Time window evaluator contributes 0 to cost (constraints only)
        return MoveDelta(0.0, feasible);
    }

    MoveDelta evaluateRemovalIncremental(
        const Tour& /*tour*/,
        const RouteCache& /*cache*/,
        const RemovalContext& /*ctx*/) const override {
        // Removal always makes TW constraints easier (less delay)
        return MoveDelta(0.0, true);
    }

    MoveDelta evaluateTwoOptIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const TwoOptContext& ctx) const override {

        auto* problem = tour.getProblem();
        if (!problem) return MoveDelta(0.0, true);

        const size_t n = tour.getNbClient();
        if (ctx.i < 0 || ctx.j < 0 ||
            ctx.i >= ctx.j || ctx.j >= static_cast<int>(n)) {
            return MoveDelta::infeasible();
        }

        // After 2-opt: segment [i+1, j] is reversed
        // Need to check:
        // 1. Feasibility of reversed segment
        // 2. Arrival at position j+1 (or depot)

        // Get departure time before reversed segment
        double departureFromPrev;
        Entity* prevNode;

        if (ctx.i < 0) {
            auto* depot = dynamic_cast<Entity*>(problem->getDepot());
            double depotOpen = 0.0;
            if (auto* tw = depot->tryGetAttribute<attributes::Rendezvous>()) {
                depotOpen = tw->getTwOpen();
            }
            departureFromPrev = depotOpen;
            prevNode = depot;
        } else {
            departureFromPrev = cache.departureTime[ctx.i];
            prevNode = dynamic_cast<Entity*>(tour.getClient(ctx.i));
        }

        // Helper to get time window
        auto getWindow = [](Entity* entity) -> std::pair<double, double> {
            double open = 0.0;
            double close = std::numeric_limits<double>::max();
            if (entity) {
                if (auto* tw = entity->tryGetAttribute<attributes::Rendezvous>()) {
                    open = tw->getTwOpen();
                    close = tw->getTwClose();
                }
            }
            return {open, close};
        };

        // Helper to get service time
        auto getService = [](Entity* entity) -> double {
            if (entity) {
                if (auto* sq = entity->tryGetAttribute<attributes::ServiceQuery>()) {
                    return sq->getService();
                }
            }
            return 0.0;
        };

        // Simulate reversed segment [j, j-1, ..., i+1]
        double currentTime = departureFromPrev;
        Entity* prev = prevNode;

        for (int k = ctx.j; k > ctx.i; --k) {
            auto* client = dynamic_cast<Entity*>(tour.getClient(k));
            if (!client) continue;

            double travel = problem->getDistanceEntity(prev, client);
            double arrival = currentTime + travel;

            auto [open, close] = getWindow(client);
            double start = std::max(arrival, open);

            if (start > close + 1e-9) {
                return MoveDelta::infeasible();
            }

            double service = getService(client);
            currentTime = start + service;
            prev = client;
        }

        // Check propagation to positions after j
        if (ctx.j + 1 < static_cast<int>(n)) {
            auto* nextClient = dynamic_cast<Entity*>(tour.getClient(ctx.j + 1));
            double travel = problem->getDistanceEntity(prev, nextClient);
            double newArrival = currentTime + travel;
            double oldArrival = cache.arrivalTime[ctx.j + 1];
            double delay = newArrival - oldArrival;

            if (delay > 1e-9 && delay > cache.maxForwardShift[ctx.j + 1] + 1e-9) {
                return MoveDelta::infeasible();
            }
        } else {
            // j is last client, check return to depot
            auto* depot = dynamic_cast<Entity*>(problem->getDepot());
            double travel = problem->getDistanceEntity(prev, depot);
            double returnTime = currentTime + travel;
            if (returnTime > cache.depotCloseTime + 1e-9) {
                return MoveDelta::infeasible();
            }
        }

        return MoveDelta(0.0, true);
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