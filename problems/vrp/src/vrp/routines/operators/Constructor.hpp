//
// Created by Sohaib LAFIFI on 22/11/2019.
//

#pragma once

#include <core/routines/operators/Constructor.hpp>
#include "../../models/Solution.hpp"
#include "../../models/Tour.hpp"

namespace vrp {
    namespace routines {
        class Constructor : public routing::Constructor {

        public:
            bool bestInsertion(routing::models::Solution *solution,
                               std::vector<routing::models::Client *> clients) override {
                while (solution->getNbTour() < solution->getProblem()->vehicles.size()) {
                    solution->pushTour(
                            new models::Tour(static_cast<Problem *>(solution->getProblem()), solution->getNbTour()));
                }
                bool insertion_found = true;
                while (!clients.empty() && insertion_found) {
                    unsigned best_t = 0, best_p = 0, best_client_i = 0;
                    insertion_found = false;
                    routing::InsertionCost *bestCost = new routing::InsertionCost(IloInfinity, true);
                    for (unsigned cc = 0; cc < clients.size(); ++cc) {
                        models::Client *client = static_cast<models::Client *>(clients[cc]);
                        for (unsigned r = 0; r < solution->getNbTour(); ++r) {
                            for (unsigned i = 0; i <= solution->getTour(r)->getNbClient(); ++i) {
                                routing::InsertionCost *cost = solution->getTour(r)->evaluateInsertion(client, i);
                                if (!cost->isPossible()) continue;
                                if (*bestCost > *cost) {
                                    insertion_found = true;
                                    best_t = r;
                                    best_p = i;
                                    best_client_i = cc;
                                    *bestCost = *cost;
                                }
                            }
                        }
                    }
                    if (insertion_found) {
                        solution->getTour(best_t)->addClient(clients[best_client_i], best_p);
                        static_cast<models::Solution *>(solution)->traveltime += bestCost->getDelta();
                        solution->notserved.erase(
                                std::remove(solution->notserved.begin(), solution->notserved.end(),
                                            clients[best_client_i]),
                                solution->notserved.end());
                        clients.erase(clients.begin() + best_client_i);

                    }
                }
                return insertion_found;

            }

        };
    }
}