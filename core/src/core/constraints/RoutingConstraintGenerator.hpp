// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/IConstraintGenerator.hpp"
#include "core/data/ComposableProblem.hpp"
#include "core/data/AttributeRegistry.hpp"
#include "core/data/attributes/GeoNode.hpp"

namespace routing {
namespace constraints {

    /**
     * @brief Base routing constraint generator
     *
     * Implements the fundamental VRP constraints:
     * - Arc decision variables (X_i_j)
     * - Vehicle affectation variables (A_i_k)
     * - Order variables for subtour elimination (o_i)
     * - Flow conservation constraints
     * - MTZ subtour elimination constraints
     *
     * This is always active as it provides the foundation for all VRP variants.
     */
    class RoutingConstraintGenerator : public IConstraintGenerator {
    public:
        std::string name() const override {
            return "RoutingConstraintGenerator";
        }

        std::vector<AttributeTypeId> requiredAttributes() const override {
            // Requires GeoNode for distance calculations
            return { std::type_index(typeid(attributes::GeoNode)) };
        }

        int priority() const override {
            return 10;  // Runs first - provides base variables
        }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            size_t n = clients.size();

            // Arc variables: X[i][j] = 1 if arc from node i to node j is used
            // Index 0 = depot, indices 1..n = clients
            problem.arcs.clear();
            for (size_t i = 0; i <= n; ++i) {
                problem.arcs.push_back(std::vector<IloNumVar>());
                for (size_t j = 0; j <= n; ++j) {
                    std::string varName = "X_" + std::to_string(i) + "_" + std::to_string(j);
                    problem.arcs.back().push_back(IloBoolVar(problem.env, varName.c_str()));

                    // No self-loops
                    if (i == j) {
                        problem.model.add(problem.arcs.back().back() == 0);
                    } else {
                        problem.model.add(problem.arcs.back().back());
                    }
                }
            }

            // Affectation variables: A[i][k] = 1 if client i is assigned to vehicle k
            auto vehicles = problem.getComposableVehicles();
            size_t m = vehicles.size();

            problem.affectation.clear();
            for (size_t i = 0; i <= n; ++i) {
                problem.affectation.push_back(std::vector<IloNumVar>());
                if (i == 0) continue;  // No affectation for depot

                for (size_t k = 0; k < m; ++k) {
                    std::string varName = "A_" + std::to_string(i) + "_" + std::to_string(k);
                    problem.affectation.back().push_back(IloBoolVar(problem.env, varName.c_str()));
                    problem.model.add(problem.affectation.back().back());
                }
            }

            // Order variables for MTZ subtour elimination
            problem.order.clear();
            for (size_t i = 0; i <= n; ++i) {
                std::string varName = "o_" + std::to_string(i);
                problem.order.push_back(IloNumVar(problem.env, 0, n, varName.c_str()));
                problem.model.add(problem.order.back());
            }
        }

        void addConstraints(ComposableProblem& problem) override {
            addAffectationConstraints(problem);
            addRoutingConstraints(problem);
            addSequenceConstraints(problem);
        }

        void addObjectiveTerms(ComposableProblem& problem, IloExpr& objExpr) override {
            auto clients = problem.getComposableClients();
            auto depots = problem.getComposableDepots();
            size_t n = clients.size();

            if (depots.empty()) return;
            auto* depot = depots[0];

            // Minimize total travel distance
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];

                // Distance from depot to client and back
                Duration distToDepot = problem.getDistance(*client, *depot);
                objExpr += distToDepot * problem.arcs[i + 1][0];  // client to depot
                objExpr += distToDepot * problem.arcs[0][i + 1];  // depot to client

                // Distance between clients
                for (size_t j = 0; j < n; ++j) {
                    if (i != j) {
                        Duration dist = problem.getDistance(*client, *clients[j]);
                        objExpr += dist * problem.arcs[i + 1][j + 1];
                    }
                }
            }
        }

    private:
        void addAffectationConstraints(ComposableProblem& problem) {
            auto clients = problem.getComposableClients();
            auto vehicles = problem.getComposableVehicles();
            size_t n = clients.size();
            size_t m = vehicles.size();

            // Each client must be assigned to exactly one vehicle
            for (size_t i = 1; i <= n; ++i) {
                IloExpr expr(problem.env);
                for (size_t k = 0; k < m; ++k) {
                    expr += problem.affectation[i][k];
                }
                problem.model.add(IloRange(problem.env, 1, expr, 1, "affectation"));
            }

            // Consistency between arcs and affectation
            for (size_t i = 1; i <= n; ++i) {
                for (size_t j = 1; j <= n; ++j) {
                    if (i == j) continue;
                    for (size_t k = 0; k < m; ++k) {
                        // If arc (i,j) is used, both must be on same vehicle
                        problem.model.add(problem.affectation[i][k] - problem.affectation[j][k]
                                          <= 1 - problem.arcs[i][j]);
                        problem.model.add(problem.affectation[j][k] - problem.affectation[i][k]
                                          <= 1 - problem.arcs[i][j]);
                        problem.model.add(problem.affectation[j][k]
                                          <= 3 - (problem.arcs[0][i] + problem.arcs[0][j]
                                                  + problem.affectation[i][k]));
                    }
                }
            }
        }

        void addRoutingConstraints(ComposableProblem& problem) {
            auto clients = problem.getComposableClients();
            auto vehicles = problem.getComposableVehicles();
            size_t n = clients.size();
            size_t m = vehicles.size();

            // Limit number of vehicles leaving depot
            IloExpr depotExpr(problem.env);
            for (size_t i = 1; i <= n; ++i) {
                depotExpr += problem.arcs[0][i];
            }
            problem.model.add(IloRange(problem.env, 0, depotExpr, IloInt(m), "routing-depot"));
            depotExpr.end();

            // Each client has exactly one outgoing arc
            for (size_t i = 1; i <= n; ++i) {
                IloExpr outExpr(problem.env);
                for (size_t j = 0; j <= n; ++j) {
                    outExpr += problem.arcs[i][j];
                }
                problem.model.add(IloRange(problem.env, 1, outExpr, 1, "routing-out"));
                outExpr.end();
            }

            // Flow conservation: in-degree = out-degree for each client
            for (size_t i = 1; i <= n; ++i) {
                IloExpr flowExpr(problem.env);
                for (size_t j = 0; j <= n; ++j) {
                    flowExpr += problem.arcs[i][j] - problem.arcs[j][i];
                }
                problem.model.add(IloRange(problem.env, 0, flowExpr, 0, "routing-flow"));
                flowExpr.end();
            }
        }

        void addSequenceConstraints(ComposableProblem& problem) {
            auto clients = problem.getComposableClients();
            size_t n = clients.size();

            // MTZ subtour elimination constraints
            for (size_t i = 1; i <= n; ++i) {
                for (size_t j = 0; j <= n; ++j) {
                    if (i == j) continue;
                    problem.model.add(problem.order[i] + 1 - problem.order[j]
                                      - n * (1 - problem.arcs[i][j]) <= 0);
                }
            }
        }
#endif
    };

    // Auto-register this generator
    ROUTING_REGISTER_GENERATOR(RoutingConstraintGenerator);

} // namespace constraints
} // namespace routing
