// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/IConstraintGenerator.hpp"
#include "core/data/ComposableProblem.hpp"
#include "core/data/AttributeRegistry.hpp"
#include "core/data/attributes/Profiter.hpp"

namespace routing {
namespace constraints {

    /**
     * @brief Profit objective generator for TOP (Team Orienteering Problem)
     *
     * Replaces the default distance minimization objective with profit maximization.
     * Only clients that are visited contribute to the objective.
     *
     * Requires: Profiter attribute (on clients)
     */
    class ProfitObjectiveGenerator : public IConstraintGenerator {
    public:
        std::string name() const override {
            return "ProfitObjectiveGenerator";
        }

        std::vector<AttributeTypeId> requiredAttributes() const override {
            return {
                std::type_index(typeid(attributes::Profiter))
            };
        }

        int priority() const override {
            return 100;  // Run after all other generators
        }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem& problem) override {
            // No additional variables needed - uses arc variables from RoutingConstraintGenerator
        }

        void addConstraints(ComposableProblem& problem) override {
            // No additional constraints needed for profit objective
        }

        void addObjectiveTerms(ComposableProblem& problem, IloExpr& objExpr) override {
            auto clients = problem.getComposableClients();
            size_t n = clients.size();

            // For profit maximization, we need to override the default objective
            // Clear the existing expression (distance minimization)
            objExpr.clear();

            // Maximize profit: sum of profits for visited clients
            // A client is visited if any outgoing arc is used
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                auto* profiter = client->tryGetAttribute<attributes::Profiter>();

                if (!profiter) continue;

                Profit profit = profiter->getProfit();

                // Client is visited if it has any outgoing arc (excluding self-loop)
                IloExpr presence(problem.env);
                for (size_t j = 0; j <= n; ++j) {
                    if (i + 1 != j) {
                        presence += problem.arcs[i + 1][j];
                    }
                }

                // Negate because CPLEX minimizes by default
                // We want to maximize profit, so minimize -profit
                objExpr -= profit * presence;
            }
        }
#endif
    };

    // Auto-register this generator
    ROUTING_REGISTER_GENERATOR(ProfitObjectiveGenerator);

} // namespace constraints
} // namespace routing
