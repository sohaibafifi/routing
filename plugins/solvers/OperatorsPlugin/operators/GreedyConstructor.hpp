// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Constructor.hpp"
#include <algorithm>
#include <limits>
#include <vector>

namespace routing {

    class GreedyConstructor : public Constructor {
    public:
        GreedyConstructor() = default;

        bool bestInsertion(Solution *solution,
                           const std::vector<models::Client *> &clients) override {
            if (!solution) {
                return false;
            }

            auto* problem = solution->getProblem();
            if (!problem) {
                return false;
            }

            ensureTours(solution, problem);

            if (clients.empty()) {
                return true;
            }

            std::vector<models::Client*> pending = clients;
            for (auto* client : pending) {
                if (!client) {
                    continue;
                }

                double bestDelta = std::numeric_limits<double>::infinity();
                InsertionCost* bestCost = nullptr;
                unsigned long bestTour = 0;
                unsigned long bestPos = 0;

                for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                    auto* tour = solution->getTour(t);
                    if (!tour) {
                        continue;
                    }
                    for (unsigned long pos = 0; pos <= tour->getNbClient(); ++pos) {
                        auto* cost = tour->evaluateInsertion(client, pos);
                        if (cost->isPossible() && cost->getDelta() < bestDelta) {
                            if (bestCost) {
                                delete bestCost;
                            }
                            bestCost = cost;
                            bestDelta = cost->getDelta();
                            bestTour = t;
                            bestPos = pos;
                        } else {
                            delete cost;
                        }
                    }
                }

                if (!bestCost) {
                    return false;
                }

                solution->addClient(bestTour, client, bestPos, bestCost);
                delete bestCost;
            }

            solution->update();
            return true;
        }

    private:
        void ensureTours(Solution* solution, Problem* problem) {
            if (solution->getNbTour() > 0) {
                return;
            }

            auto vehicles = problem->getComposableVehicles();
            size_t numVehicles = vehicles.size();
            if (numVehicles == 0) {
                numVehicles = 1;
            }

            for (size_t i = 0; i < numVehicles; ++i) {
                solution->pushTour(new Tour(problem, static_cast<unsigned>(i)));
            }
        }
    };

}
