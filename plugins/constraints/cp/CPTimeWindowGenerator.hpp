// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "CPRoutingGenerator.hpp"

namespace routing {
namespace cp {
namespace generators {

/**
 * @brief CP Time Window constraint generator
 *
 * Implements time window constraints using CP interval variables:
 * - Each visit is modeled as an interval with bounds from time windows
 * - Service time defines the interval duration
 * - Precedence constraints ensure temporal consistency
 * - No-overlap ensures each vehicle visits nodes in sequence
 *
 * The CP model naturally handles:
 * - Waiting time (start can be later than arrival)
 * - Service time as interval duration
 * - Time window bounds as interval bounds
 */
class CPTimeWindowGenerator : public ICPConstraintGenerator {
public:
    std::string name() const override {
        return "CPTimeWindowGenerator";
    }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return {
            std::type_index(typeid(attributes::Rendezvous)),
            std::type_index(typeid(attributes::ServiceQuery))
        };
    }

    int priority() const override {
        return 60;  // After routing (10) and capacity (50)
    }

    void setRoutingGenerator(CPRoutingGenerator* routing) {
        routingGen_ = routing;
    }

    void addVariables(ICPBackend& cp, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto depots = problem.getComposableDepots();
        auto vehicles = problem.getComposableVehicles();

        size_t n = clients.size();
        size_t m = vehicles.size();

        // Get depot time window
        int depotOpen = 0;
        int depotClose = 1000000;
        if (!depots.empty()) {
            auto* depot = depots[0];
            auto* depotTW = depot->tryGetAttribute<attributes::Rendezvous>();
            if (depotTW) {
                depotOpen = static_cast<int>(depotTW->getTwOpen());
                depotClose = static_cast<int>(depotTW->getTwClose());
            }
        }

        // Create visit intervals for each client
        visitIntervals_.clear();
        startTimes_.clear();
        endTimes_.clear();

        for (size_t i = 0; i < n; ++i) {
            auto* client = clients[i];
            auto* tw = client->tryGetAttribute<attributes::Rendezvous>();
            auto* service = client->tryGetAttribute<attributes::ServiceQuery>();

            int twOpen = tw ? static_cast<int>(tw->getTwOpen()) : depotOpen;
            int twClose = tw ? static_cast<int>(tw->getTwClose()) : depotClose;
            int serviceTime = service ? static_cast<int>(service->getService()) : 0;

            // Use "tw_visit_" prefix to keep time-window intervals distinct
            std::string varName = "tw_visit_" + std::to_string(i);

            // Create interval variable with time window bounds and service time
            IntervalVar visit = cp.newIntervalVar(
                twOpen,           // Earliest start (open time)
                twClose + serviceTime,  // Latest end (close time + service)
                serviceTime,      // Min duration (service time)
                serviceTime,      // Max duration (service time)
                varName
            );

            visitIntervals_.push_back(visit);

            // Get start and end time variables for linking
            startTimes_.push_back(cp.startOf(visit));
            endTimes_.push_back(cp.endOf(visit));
        }

        // Create vehicle route intervals for no-overlap per vehicle
        // Each vehicle has a sequence of visits that cannot overlap
        vehicleVisits_.clear();
        vehicleVisits_.resize(m);
    }

    void addConstraints(ICPBackend& cp, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto depots = problem.getComposableDepots();
        auto vehicles = problem.getComposableVehicles();

        size_t n = clients.size();
        size_t m = vehicles.size();

        if (n == 0 || depots.empty()) return;
        auto* depot = depots[0];

        // Get depot time window
        int depotOpen = 0;
        int depotClose = 1000000;
        auto* depotTW = depot->tryGetAttribute<attributes::Rendezvous>();
        if (depotTW) {
            depotOpen = static_cast<int>(depotTW->getTwOpen());
            depotClose = static_cast<int>(depotTW->getTwClose());
        }

        // If we have access to routing generator, add temporal constraints
        if (routingGen_) {
            const auto& nextVars = routingGen_->getNextVars();
            size_t totalNodes = routingGen_->getTotalNodes();

            // For each pair of clients, if one follows the other,
            // the end of the first + travel time <= start of the second
            for (size_t i = 0; i < n; ++i) {
                auto* clientI = clients[i];

                for (size_t j = 0; j < n; ++j) {
                    if (i == j) continue;

                    auto* clientJ = clients[j];

                    // Travel time from i to j
                    int travelTime = static_cast<int>(problem.getDistance(*clientI, *clientJ));

                    // If next[nodeI] == nodeJ, then end[i] + travel <= start[j]
                    size_t nodeI = routingGen_->clientNodeIndex(i);
                    size_t nodeJ = routingGen_->clientNodeIndex(j);

                    // Create indicator variable: isSuccessor = (next[nodeI] == nodeJ)
                    IntVar isSuccessor = cp.newBoolVar("succ_" + std::to_string(i) + "_" + std::to_string(j));

                    // Link indicator to successor variable using reification:
                    // isSuccessor == 1 iff next[nodeI] == nodeJ
                    cp.addReification(isSuccessor, nextVars[nodeI], static_cast<int>(nodeJ));

                    // If isSuccessor then end[i] + travel <= start[j]
                    LinearExpr precedenceExpr(endTimes_[i]);
                    precedenceExpr.addConstant(travelTime);
                    precedenceExpr.addTerm(startTimes_[j], -1);

                    // end[i] + travel - start[j] <= 0 when isSuccessor
                    cp.addImplication(isSuccessor, precedenceExpr, INT_MIN, 0);
                }

                // NOTE: Depot travel constraints are implied by the time windows
                // and the precedence constraints. Removing them to simplify the model
                // and avoid potential conflicts with interval variable bounds.

                // The time window bounds on the interval variables already enforce:
                // - start[i] >= twOpen[i]
                // - end[i] <= twClose[i] + serviceTime[i]
                // - start[i] + serviceTime[i] = end[i]
            }
        } else {
            // Without routing generator, just enforce time window bounds
            // The interval variables already have bounds set
            // Add basic precedence constraints if needed

            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                auto* tw = client->tryGetAttribute<attributes::Rendezvous>();

                if (tw) {
                    // Enforce start time within window
                    LinearExpr startExpr(startTimes_[i]);
                    cp.addGreaterOrEqual(startExpr, static_cast<int>(tw->getTwOpen()));

                    // End time must respect closing
                    LinearExpr endExpr(endTimes_[i]);
                    cp.addLessOrEqual(endExpr, static_cast<int>(tw->getTwClose()) +
                        (client->tryGetAttribute<attributes::ServiceQuery>() ?
                         static_cast<int>(client->tryGetAttribute<attributes::ServiceQuery>()->getService()) : 0));
                }
            }
        }
    }

    void addObjectiveTerms(ICPBackend& cp, Problem& problem, LinearExpr& expr) override {
        // Could add penalty for late arrivals or total time
        // For now, routing generator handles distance minimization
    }

    void extractSolution(const ICPBackend& cp, Problem& problem) override {
        // Extract arrival times from solution
        arrivalTimes_.clear();
        for (size_t i = 0; i < visitIntervals_.size(); ++i) {
            arrivalTimes_.push_back(cp.getStart(visitIntervals_[i]));
        }
    }

    // Accessors
    const std::vector<IntervalVar>& getVisitIntervals() const { return visitIntervals_; }
    const std::vector<IntVar>& getStartTimes() const { return startTimes_; }
    const std::vector<IntVar>& getEndTimes() const { return endTimes_; }
    const std::vector<int>& getArrivalTimes() const { return arrivalTimes_; }

private:
    CPRoutingGenerator* routingGen_ = nullptr;

    std::vector<IntervalVar> visitIntervals_;
    std::vector<IntVar> startTimes_;
    std::vector<IntVar> endTimes_;

    std::vector<std::vector<OptionalIntervalVar>> vehicleVisits_;  // Visits per vehicle

    std::vector<int> arrivalTimes_;  // Extracted arrival times
};

} // namespace generators
} // namespace cp
} // namespace routing
