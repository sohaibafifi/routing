//
// Created by ali on 3/28/19.
//

#include "Constructor.hpp"



bool CVRPTW::Constructor::bestInsertion(routing::models::Solution *solution) {
    //build empty tours for each unused vechicle

    while (solution->getNbTour() < solution->getProblem()->vehicles.size()) {
        solution->pushTour(new Tour(static_cast<Problem *>(solution->getProblem()), solution->getNbTour(), static_cast<CVRPTW::Solution*>(solution)->visits));
    }


    //tells whether a new best_insertion position has been found
    //true at beginning to begin loop
    bool insertion_found = true;

    //loop at each unrouted client
    while (!solution->notserved.empty() && insertion_found) {
        unsigned best_t = 0, best_p = 0, best_client_i = 0;
        insertion_found = false;
        InsertionCost* bestCost = new InsertionCost( IloInfinity,true,IloInfinity);
        //InsertionCost* cost = new InsertionCost(std::numeric_limits<routing::Duration >::max(),true,std::numeric_limits<routing::Duration >::max());
        double shift_i = 0.0;
        //foreach client
        for (unsigned cc = 0; cc < solution->notserved.size(); ++cc) {
            Client *client = static_cast<Client *>(solution->notserved[cc]);
            //foreach tour
            for (unsigned r = 0; r < static_cast<Solution *>(solution)->getNbTour(); ++r) {
                //foreach position
                for (unsigned i = 0;
                     i <= int(static_cast<Tour *>(static_cast<Solution *>(solution)->getTour(r))->getNbClient()); ++i) {


                    CVRPTW::InsertionCost* cost = static_cast<CVRPTW::InsertionCost*>(static_cast<Tour *>(static_cast<Solution *>(solution)->getTour(
                            r))->evaluateInsertion(client, i));


                    if (!cost->isPossible()) continue;
                    //TODO: Implement operator > for InsertionCost Class
                    bool res = bestCost->getDelta() > cost->getDelta();
                    if (res) {
                        insertion_found = true;
                        best_t = r;
                        best_p = i;
                        best_client_i = cc;
                        *bestCost = *cost;
                    }
                }
            }
        }
        if (insertion_found) { //update solution

            static_cast<Solution*>(solution)->getTour(best_t)->addClient(static_cast<Solution*>(solution)->notserved[best_client_i], best_p );
            static_cast<Solution*>(solution)->traveltime += bestCost->getDelta();
            static_cast<Solution*>(solution)->notserved.erase(static_cast<Solution*>(solution)->notserved.begin() + best_client_i);
            static_cast<Solution*>(solution)->update();
            static_cast<Solution*>(solution)->updateTimeVariables(static_cast<Solution*>(solution)->getTour(best_t),best_p,bestCost->getShift());
        }

    }
    return insertion_found;

}