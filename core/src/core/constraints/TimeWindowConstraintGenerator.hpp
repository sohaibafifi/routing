// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/IConstraintGenerator.hpp"
#include "core/data/ComposableProblem.hpp"
#include "core/data/AttributeRegistry.hpp"
#include "core/data/attributes/Rendezvous.hpp"
#include "core/data/attributes/ServiceQuery.hpp"
#include "core/data/attributes/GeoNode.hpp"

namespace routing {
namespace constraints {

    /**
     * @brief Time window constraint generator
     *
     * Implements time window constraints:
     * - Start time variables (t_i) for each node
     * - Time window bounds (open/close)
     * - Sequence constraints with service times and travel times
     *
     * Requires: Rendezvous (time windows) and ServiceQuery (service times)
     */
    class TimeWindowConstraintGenerator : public IConstraintGenerator {
    public:
        std::string name() const override {
            return "TimeWindowConstraintGenerator";
        }

        std::vector<AttributeTypeId> requiredAttributes() const override {
            return {
                std::type_index(typeid(attributes::Rendezvous)),
                std::type_index(typeid(attributes::ServiceQuery))
            };
        }

        int priority() const override {
            return 60;  // After capacity (50)
        }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto depots = problem.getComposableDepots();
            size_t n = clients.size();

            startTime_.clear();

            // Start time for depot
            if (!depots.empty()) {
                auto* depot = depots[0];
                auto* depotTW = depot->tryGetAttribute<attributes::Rendezvous>();
                if (depotTW) {
                    startTime_.push_back(IloNumVar(problem.env,
                        depotTW->getTwOpen(),
                        depotTW->getTwClose(),
                        "t_0"));
                } else {
                    startTime_.push_back(IloNumVar(problem.env, 0, 1e9, "t_0"));
                }
                problem.model.add(startTime_.back());
            }

            // Start time for each client
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                auto* tw = client->tryGetAttribute<attributes::Rendezvous>();

                std::string varName = "t_" + std::to_string(i + 1);
                if (tw) {
                    startTime_.push_back(IloNumVar(problem.env,
                        tw->getTwOpen(),
                        tw->getTwClose(),
                        varName.c_str()));
                } else {
                    startTime_.push_back(IloNumVar(problem.env, 0, 1e9, varName.c_str()));
                }
                problem.model.add(startTime_.back());
            }

            // Store references in problem for other generators (e.g., SyncConstraintGenerator)
            problem.startTime = startTime_;
        }

        void addConstraints(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto depots = problem.getComposableDepots();
            size_t n = clients.size();

            if (depots.empty()) return;
            auto* depot = depots[0];
            auto* depotTW = depot->tryGetAttribute<attributes::Rendezvous>();

            // Minimum start time based on travel from depot
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                Duration distFromDepot = problem.getDistance(*client, *depot);
                problem.model.add(startTime_[i + 1] >= distFromDepot);
            }

            // Time window propagation constraints
            for (size_t i = 1; i <= n; ++i) {
                auto* clientI = clients[i - 1];
                auto* twI = clientI->tryGetAttribute<attributes::Rendezvous>();
                auto* serviceI = clientI->tryGetAttribute<attributes::ServiceQuery>();

                if (!twI || !serviceI) continue;

                Duration serviceTime = serviceI->getService();
                Duration twCloseI = twI->getTwClose();

                for (size_t j = 0; j <= n; ++j) {
                    if (i == j) continue;

                    Duration travelTime;
                    Duration twOpenJ;

                    if (j == 0) {
                        // Arc to depot
                        travelTime = problem.getDistance(*clientI, *depot);
                        twOpenJ = depotTW ? depotTW->getTwOpen() : 0;
                    } else {
                        // Arc to another client
                        auto* clientJ = clients[j - 1];
                        auto* twJ = clientJ->tryGetAttribute<attributes::Rendezvous>();
                        if (!twJ) continue;

                        travelTime = problem.getDistance(*clientI, *clientJ);
                        twOpenJ = twJ->getTwOpen();
                    }

                    // If arc (i,j) is used: start[i] + service[i] + travel[i,j] <= start[j]
                    // With big-M for linearization
                    Duration bigM = twCloseI - twOpenJ;

                    problem.model.add(
                        startTime_[i] + (serviceTime + travelTime) * problem.arcs[i][j]
                        <= startTime_[j] + bigM * (1 - problem.arcs[i][j])
                    );
                }
            }
        }

    private:
        std::vector<IloNumVar> startTime_;
#endif
    };

    // Auto-register this generator
    ROUTING_REGISTER_GENERATOR(TimeWindowConstraintGenerator);

} // namespace constraints
} // namespace routing
