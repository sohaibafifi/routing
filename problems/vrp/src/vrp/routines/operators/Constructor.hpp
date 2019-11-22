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
            bool bestInsertion(routing::models::Solution *solution) override {
                while (solution->getNbTour() < solution->getProblem()->vehicles.size()) {
                    solution->pushTour(
                            new models::Tour(static_cast<Problem *>(solution->getProblem()), solution->getNbTour()));
                }
                bool insertion_found = true;
                while (!solution->notserved.empty() && insertion_found) {
                    unsigned best_t = 0, best_p = 0, best_client_i = 0;
                    insertion_found = false;
                    routing::InsertionCost *bestCost = new routing::InsertionCost(IloInfinity, true);
                    for (unsigned cc = 0; cc < solution->notserved.size(); ++cc) {
                        models::Client *client = static_cast<models::Client *>(solution->notserved[cc]);
                        for (unsigned r = 0; r < static_cast<models::Solution *>(solution)->getNbTour(); ++r) {
                            for (unsigned i = 0; i <=
                                                 int(static_cast<models::Tour *>(static_cast<models::Solution *>(solution)->getTour(
                                                         r))->getNbClient()); ++i) {
                                //bool possible = true;
                                routing::InsertionCost *cost = static_cast<routing::InsertionCost *>(static_cast<models::Tour *>(static_cast<models::Solution *>(solution)->getTour(
                                        r))->evaluateInsertion(client, i));
                                if (!cost->isPossible()) continue;
                                if (bestCost->getDelta() > cost->getDelta()) {
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
                        static_cast<models::Solution *>(solution)->getTour(best_t)->addClient(
                                static_cast<models::Solution *>(solution)->notserved[best_client_i], best_p);
                        static_cast<models::Solution *>(solution)->traveltime += bestCost->getDelta();
                        static_cast<models::Solution *>(solution)->notserved.erase(
                                static_cast<models::Solution *>(solution)->notserved.begin() + best_client_i);

                    }
                }
                return insertion_found;
            }
        };
    }
}