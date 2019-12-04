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
                               const std::vector<routing::models::Client *> p_clients) override {
                std::vector<routing::models::Client *> clients = p_clients;

                bool insertion_found = true;
                while (!clients.empty() && insertion_found) {
                    unsigned best_t = 0, best_p = 0, best_client_i = 0;
                    insertion_found = false;
                    auto *bestCost = new routing::InsertionCost(std::numeric_limits<routing::Duration>::max(), true);
                    for (unsigned cc = 0; cc < clients.size(); ++cc) {
                        auto *client = dynamic_cast<models::Client *>(clients[cc]);
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
                        solution->addClient(best_t, clients[best_client_i], best_p);
                        //dynamic_cast<models::Solution *>(solution)->traveltime += bestCost->getDelta();
                        clients.erase(clients.begin() + best_client_i);
                        solution->getTour(best_t)->update();
                    }
                }
                return insertion_found;

            }

        };
    }
}