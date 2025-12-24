// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/IConstraintGenerator.hpp"
#include "core/data/ComposableProblem.hpp"
#include "core/data/AttributeRegistry.hpp"
#include "core/data/attributes/Consumer.hpp"
#include "core/data/attributes/Stock.hpp"

namespace routing {
namespace constraints {

    /**
     * @brief Capacity constraint generator
     *
     * Implements vehicle capacity constraints:
     * - Consumption variables (q_i) for tracking load
     * - Vehicle capacity limits per route
     * - MTZ-style load propagation constraints
     *
     * Requires: Consumer (on clients) and Stock (on vehicles)
     */
    class CapacityConstraintGenerator : public IConstraintGenerator {
    public:
        std::string name() const override {
            return "CapacityConstraintGenerator";
        }

        std::vector<AttributeTypeId> requiredAttributes() const override {
            return {
                std::type_index(typeid(attributes::Consumer)),
                std::type_index(typeid(attributes::Stock))
            };
        }

        int priority() const override {
            return 50;  // After routing (10), before time windows (60)
        }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto vehicles = problem.getComposableVehicles();
            size_t n = clients.size();

            if (vehicles.empty()) return;
            auto* stock = vehicles[0]->tryGetAttribute<attributes::Stock>();
            if (!stock) return;

            consumption_.clear();

            // Consumption variable for depot (always 0)
            consumption_.push_back(IloNumVar(problem.env, 0, 0, "q_0"));
            problem.model.add(consumption_.back());

            // Consumption variables for each client
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                auto* consumer = client->tryGetAttribute<attributes::Consumer>();
                if (!consumer) {
                    // Client without demand - create dummy variable
                    std::string varName = "q_" + std::to_string(i + 1);
                    consumption_.push_back(IloNumVar(problem.env, 0, stock->getCapacity(), varName.c_str()));
                } else {
                    std::string varName = "q_" + std::to_string(i + 1);
                    Demand demand = consumer->getDemand();
                    Capacity capacity = stock->getCapacity();

                    consumption_.push_back(IloNumVar(problem.env,
                        std::max(demand, (Demand)0),
                        std::min(capacity, capacity + demand),
                        varName.c_str()));
                }
                problem.model.add(consumption_.back());
            }
        }

        void addConstraints(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto vehicles = problem.getComposableVehicles();
            size_t n = clients.size();
            size_t m = vehicles.size();

            // Vehicle capacity per route constraint
            for (size_t k = 0; k < m; ++k) {
                auto* stock = vehicles[k]->tryGetAttribute<attributes::Stock>();
                if (!stock) continue;

                IloExpr expr(problem.env);
                for (size_t i = 0; i < n; ++i) {
                    auto* consumer = clients[i]->tryGetAttribute<attributes::Consumer>();
                    if (consumer) {
                        expr += consumer->getDemand() * problem.affectation[i + 1][k];
                    }
                }
                problem.model.add(expr <= stock->getCapacity());
            }

            // MTZ-style load propagation constraints
            if (vehicles.empty()) return;
            auto* stock = vehicles[0]->tryGetAttribute<attributes::Stock>();
            if (!stock) return;

            for (size_t i = 1; i <= n; ++i) {
                for (size_t j = 1; j <= n; ++j) {
                    if (i == j) continue;

                    auto* consumerJ = clients[j - 1]->tryGetAttribute<attributes::Consumer>();
                    if (!consumerJ) continue;

                    Demand demandJ = consumerJ->getDemand();
                    Capacity capacity = stock->getCapacity();

                    // If arc (i,j) is used, load at j must account for demand at j
                    problem.model.add(
                        consumption_[i] + demandJ
                        <= consumption_[j] + (capacity + demandJ) * (1 - problem.arcs[i][j])
                    );
                }
            }
        }

    private:
        std::vector<IloNumVar> consumption_;
#endif
    };

    // Auto-register this generator
    ROUTING_REGISTER_GENERATOR(CapacityConstraintGenerator);

} // namespace constraints
} // namespace routing
