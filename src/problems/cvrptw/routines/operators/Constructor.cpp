//
// Created by ali on 3/28/19.
//

#include "Constructor.h"



bool CVRPTW::Constructor::bestInsertion(routing::models::Solution *solution) {
    //build empty tours for each unused vechicle
    while (solution->getNbTour() < solution->getProblem()->vehicles.size()) {
        solution->pushTour(new Tour(static_cast<Problem *>(solution->getProblem()), solution->getNbTour()));
    }

    //tells whether a new best_insertion position has been found
    //true at beginning to begin loop
    bool insertion_found = true;

    //loop at each unrouted client
    while (!solution->notserved.empty() && insertion_found) {

        unsigned best_t = 0, best_p = 0, best_client_i = 0;
        insertion_found = false;
        routing::Duration bestCost = IloInfinity;
        double shift_i = 0.0;
        //foreach client
        for (unsigned cc = 0; cc < solution->notserved.size(); ++cc) {
            Client *client = static_cast<Client *>(solution->notserved[cc]);
            //foreach tour
            for (unsigned r = 0; r < static_cast<Solution *>(solution)->getNbTour(); ++r) {
                //foreach position
                for (unsigned i = 0;
                     i <= int(static_cast<Tour *>(static_cast<Solution *>(solution)->getTour(r))->getNbClient()); ++i) {

                    bool possible = true;
                    std::pair<routing::Duration,double> cost = static_cast<Tour *>(static_cast<Solution *>(solution)->getTour(
                            r))->evaluateInsertion(client, i, possible, static_cast<CVRPTW::Solution*>(solution)->visits);
                    routing::Duration delta = cost.first;
                    shift_i = cost.second;
                    if (!possible) continue;
                    if (bestCost > delta) {
                        insertion_found = true;
                        best_t = r;
                        best_p = i;
                        best_client_i = cc;
                        bestCost = delta;
                    }
                }
            }
        }
        if (insertion_found) { //update solution
            static_cast<Solution*>(solution)->getTour(best_t)->addClient(static_cast<Solution*>(solution)->notserved[best_client_i], best_p );
            static_cast<Solution*>(solution)->traveltime += bestCost;
            static_cast<Solution*>(solution)->notserved.erase(static_cast<Solution*>(solution)->notserved.begin() + best_client_i);
            static_cast<Solution*>(solution)->update();
        }
    }
    return insertion_found;

}