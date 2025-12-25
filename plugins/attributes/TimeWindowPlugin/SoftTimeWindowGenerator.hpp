// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/ComposableProblem.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"

namespace routing {
namespace constraints {

    /**
     * @brief Soft time window constraint generator for CVRPSTW
     *
     * Unlike hard time windows, soft time windows allow violations
     * with penalties:
     * - Wait penalty: for arriving before the time window opens
     * - Delay penalty: for arriving after the time window closes
     *
     * The objective is lexicographic:
     * 1. Minimize total wait + delay penalty
     * 2. Minimize travel distance
     *
     * This generator conflicts with TimeWindowConstraintGenerator
     * since it provides alternative (soft) time window handling.
     *
     * Requires: Rendezvous (time windows) and ServiceQuery (service times)
     */
    class SoftTimeWindowGenerator : public IConstraintGenerator {
    public:
        SoftTimeWindowGenerator(double waitPenalty = 1.0, double delayPenalty = 1.0)
            : waitPenalty_(waitPenalty), delayPenalty_(delayPenalty) {}

        std::string name() const override {
            return "SoftTimeWindowGenerator";
        }

        std::vector<AttributeTypeId> requiredAttributes() const override {
            return {
                std::type_index(typeid(attributes::Rendezvous)),
                std::type_index(typeid(attributes::ServiceQuery))
            };
        }

        int priority() const override {
            return 65;  // After capacity (50), slightly after hard TW (60)
        }

        void setWaitPenalty(double penalty) { waitPenalty_ = penalty; }
        void setDelayPenalty(double penalty) { delayPenalty_ = penalty; }
        double getWaitPenalty() const { return waitPenalty_; }
        double getDelayPenalty() const { return delayPenalty_; }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto depots = problem.getComposableDepots();
            size_t n = clients.size();

            startTime_.clear();
            wait_.clear();
            delay_.clear();

            // Variables for depot
            startTime_.push_back(IloNumVar(problem.env, 0, IloInfinity, "s_0"));
            wait_.push_back(IloNumVar(problem.env, 0, IloInfinity, "w_0"));
            delay_.push_back(IloNumVar(problem.env, 0, IloInfinity, "d_0"));

            problem.model.add(startTime_.back());
            problem.model.add(wait_.back());
            problem.model.add(delay_.back());

            // Depot soft TW constraints
            if (!depots.empty()) {
                auto* depot = depots[0];
                auto* depotTW = depot->tryGetAttribute<attributes::Rendezvous>();
                if (depotTW) {
                    // wait >= tw_open - start (early arrival penalty)
                    problem.model.add(wait_.back() >= depotTW->getTwOpen() - startTime_.back());
                    // delay >= start - tw_close (late arrival penalty)
                    problem.model.add(delay_.back() >= startTime_.back() - depotTW->getTwClose());
                }
            }

            // Variables for each client
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                auto* tw = client->tryGetAttribute<attributes::Rendezvous>();

                std::string startName = "s_" + std::to_string(i + 1);
                std::string waitName = "w_" + std::to_string(i + 1);
                std::string delayName = "d_" + std::to_string(i + 1);

                startTime_.push_back(IloNumVar(problem.env, 0, IloInfinity, startName.c_str()));
                wait_.push_back(IloNumVar(problem.env, 0, IloInfinity, waitName.c_str()));
                delay_.push_back(IloNumVar(problem.env, 0, IloInfinity, delayName.c_str()));

                problem.model.add(startTime_.back());
                problem.model.add(wait_.back());
                problem.model.add(delay_.back());

                // Soft TW constraints for this client
                if (tw) {
                    // wait >= tw_open - start (early arrival penalty)
                    problem.model.add(wait_.back() >= tw->getTwOpen() - startTime_.back());
                    // delay >= start - tw_close (late arrival penalty)
                    problem.model.add(delay_.back() >= startTime_.back() - tw->getTwClose());
                }
            }

            // Store references in problem for SyncConstraintGenerator to use
            problem.startTime = startTime_;
        }

        void addConstraints(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto depots = problem.getComposableDepots();
            size_t n = clients.size();

            if (depots.empty()) return;
            auto* depot = depots[0];

            // Time propagation constraints (like hard TW but without bounds)
            for (size_t i = 1; i <= n; ++i) {
                auto* clientI = clients[i - 1];
                auto* serviceI = clientI->tryGetAttribute<attributes::ServiceQuery>();

                Duration serviceTime = serviceI ? serviceI->getService() : 0;

                for (size_t j = 0; j <= n; ++j) {
                    if (i == j) continue;

                    Duration travelTime;
                    if (j == 0) {
                        // Arc to depot
                        travelTime = problem.getDistance(*clientI, *depot);
                    } else {
                        // Arc to another client
                        auto* clientJ = clients[j - 1];
                        travelTime = problem.getDistance(*clientI, *clientJ);
                    }

                    // Big-M formulation for time propagation
                    // If arc (i,j) is used: start[j] >= start[i] + service[i] + travel[i,j]
                    Duration bigM = 1e9;  // Large constant

                    problem.model.add(
                        startTime_[j] >= startTime_[i] + serviceTime + travelTime
                                       - bigM * (1 - problem.arcs[i][j])
                    );
                }
            }

            // Arcs from depot
            for (size_t j = 1; j <= n; ++j) {
                auto* clientJ = clients[j - 1];
                Duration travelTime = problem.getDistance(*depot, *clientJ);
                Duration bigM = 1e9;

                problem.model.add(
                    startTime_[j] >= startTime_[0] + travelTime
                                   - bigM * (1 - problem.arcs[0][j])
                );
            }
        }

        void addObjectiveTerms(ComposableProblem& problem, IloExpr& objExpr) override {
            // Add penalty terms to the objective
            // Note: The SoftTimeWindow problem uses lexicographic optimization
            // where penalty minimization comes before distance minimization.
            // For simplicity here, we add weighted penalties to the existing objective.

            for (size_t i = 0; i < wait_.size(); ++i) {
                objExpr += waitPenalty_ * wait_[i];
                objExpr += delayPenalty_ * delay_[i];
            }
        }

    private:
        std::vector<IloNumVar> startTime_;
        std::vector<IloNumVar> wait_;
        std::vector<IloNumVar> delay_;

        double waitPenalty_;
        double delayPenalty_;
#endif
    };

    // Note: Not auto-registering this generator because it conflicts with
    // TimeWindowConstraintGenerator. Users should explicitly choose soft vs hard TW.

} // namespace constraints
} // namespace routing
