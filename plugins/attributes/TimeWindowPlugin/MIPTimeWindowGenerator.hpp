// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IMIPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/RoutingPlugin/MIPRoutingGenerator.hpp"

#include <vector>
#include <limits>

namespace routing {
namespace mip {
namespace generators {

/**
 * @brief MIP Time Window constraint generator
 *
 * Implements time window constraints using continuous time variables:
 * - t[i] = arrival time at node i
 * - Time window bounds: tw_open[i] <= t[i] <= tw_close[i]
 * - Precedence constraints: if x[i][j] = 1, then t[i] + service[i] + travel[i][j] <= t[j]
 * - Uses big-M formulation with indicator constraints
 */
class MIPTimeWindowGenerator : public IMIPConstraintGenerator {
public:
    std::string name() const override {
        return "MIPTimeWindowGenerator";
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

    void setRoutingGenerator(MIPRoutingGenerator* routing) {
        routingGen_ = routing;
    }

    void addVariables(IMIPBackend& mip, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto depots = problem.getComposableDepots();

        if (depots.empty()) return;

        size_t n = clients.size();

        // Get depot time window
        auto* depot = depots[0];
        auto* depotTW = depot->tryGetAttribute<attributes::Rendezvous>();
        double depotOpen = 0.0;
        double depotClose = 1000000.0;
        if (depotTW) {
            depotOpen = depotTW->getTwOpen();
            depotClose = depotTW->getTwClose();
        }

        // Create time variables for each node (depot + clients)
        timeVars_.clear();
        timeVars_.resize(n + 1);

        // Depot time variable
        timeVars_[0] = mip.newVar(depotOpen, depotClose, "t_0");
        depotEndVar_ = mip.newVar(depotOpen, depotClose, "t_end");

        // Client time variables
        for (size_t i = 0; i < n; ++i) {
            auto* client = clients[i];
            auto* tw = client->tryGetAttribute<attributes::Rendezvous>();

            double twOpen = tw ? tw->getTwOpen() : depotOpen;
            double twClose = tw ? tw->getTwClose() : depotClose;

            timeVars_[i + 1] = mip.newVar(twOpen, twClose, "t_" + std::to_string(i + 1));
        }
    }

    void addConstraints(IMIPBackend& mip, Problem& problem) override {
        if (!routingGen_) return;  // Need routing generator for arc variables

        auto clients = problem.getComposableClients();
        auto depots = problem.getComposableDepots();

        if (depots.empty()) return;

        size_t n = clients.size();
        auto* depot = depots[0];

        const auto& arcVars = routingGen_->getArcVars();

        // Big-M value (should be large enough to not constrain, but not too large for numerical stability)
        double M = 1000000.0;

        // Precedence constraints with big-M
        // If x[i][j] = 1, then t[i] + service[i] + travel[i][j] <= t[j]
        // Reformulation: t[i] + service[i] + travel[i][j] - t[j] <= M * (1 - x[i][j])

        // Ensure the depot start time is not after the latest return time.
        if (depotEndVar_.isValid()) {
            LinearExpr depotOrder;
            depotOrder.addTerm(timeVars_[0], 1.0);
            depotOrder.addTerm(depotEndVar_, -1.0);
            mip.addLessEqual(depotOrder, 0.0, "depot_time_order");
        }

        for (size_t i = 0; i <= n; ++i) {
            // Get service time for node i
            double serviceI = 0.0;
            if (i > 0) {
                auto* client = clients[i - 1];
                auto* svc = client->tryGetAttribute<attributes::ServiceQuery>();
                if (svc) serviceI = svc->getService();
            }

            for (size_t j = 0; j <= n; ++j) {
                if (i == j) continue;

                // Calculate travel time from i to j
                double travelTime = 0.0;
                if (i == 0 && j > 0) {
                    // Depot to client
                    travelTime = problem.getDistanceEntity(depot, clients[j - 1]);
                } else if (i > 0 && j == 0) {
                    // Client to depot
                    travelTime = problem.getDistanceEntity(clients[i - 1], depot);
                } else if (i > 0 && j > 0) {
                    // Client to client
                    travelTime = problem.getDistanceEntity(clients[i - 1], clients[j - 1]);
                }

                // Precedence constraint with big-M
                // t[i] + service[i] + travel[i][j] - t[j] <= M * (1 - x[i][j])
                // Equivalent to: t[i] - t[j] + M * x[i][j] <= M - service[i] - travel[i][j]
                LinearExpr precedence;
                const Var timeI = (i == 0) ? timeVars_[0] : timeVars_[i];
                const Var timeJ = (j == 0 && depotEndVar_.isValid()) ? depotEndVar_ : timeVars_[j];
                precedence.addTerm(timeI, 1.0);
                precedence.addTerm(timeJ, -1.0);
                precedence.addTerm(arcVars[i][j], M);

                mip.addLessEqual(precedence, M - serviceI - travelTime,
                    "prec_" + std::to_string(i) + "_" + std::to_string(j));
            }
        }
    }

    void addObjectiveTerms(IMIPBackend& mip, Problem& problem, LinearExpr& expr) override {
        // Time windows don't contribute to objective in standard VRPTW
        // Could add penalties for tardiness or total time if needed
    }

    void extractSolution(const IMIPBackend& mip, Problem& problem, Solution& solution) override {
        // Extract arrival times from solution
        arrivalTimes_.clear();
        for (size_t i = 0; i < timeVars_.size(); ++i) {
            arrivalTimes_.push_back(mip.getValue(timeVars_[i]));
        }
    }

    // Accessors
    const std::vector<Var>& getTimeVars() const { return timeVars_; }
    const std::vector<double>& getArrivalTimes() const { return arrivalTimes_; }

private:
    MIPRoutingGenerator* routingGen_ = nullptr;
    std::vector<Var> timeVars_;           // Time variables t[i]
    std::vector<double> arrivalTimes_;    // Extracted arrival times
    Var depotEndVar_;                     // Latest return time at depot
};

} // namespace generators
} // namespace mip
} // namespace routing
