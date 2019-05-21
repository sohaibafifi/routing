//
// Created by ali on 5/10/19.
//

#include "Diver.hpp"
#include "../../models/Solution.hpp"

#include <ilcplex/ilocplexi.h>
#include "../../../../Utility.hpp"

bool CVRPTW::Diver::dive(routing::models::Solution* sol, routing::forbiddenPositions* fp)
{
    //routing::forbiddenPositions* fp = extractForbiddenPositions(problem);
    CVRPTW::Solution* solution = static_cast<CVRPTW::Solution*>(sol);


    //tells whether a new best_insertion position has been found
    //true at beginning to begin loop
    bool insertion_found = true;

    //loop at each unrouted client
    while (!solution->toRoute.empty() && insertion_found) {

        unsigned best_t = 0, best_p = 0, best_client_i = 0;
        insertion_found = false;
        InsertionCost* bestCost = new InsertionCost( IloInfinity,true,IloInfinity);
        //InsertionCost* cost = new InsertionCost(std::numeric_limits<routing::Duration >::max(),true,std::numeric_limits<routing::Duration >::max());
        double shift_i = 0.0;
        //foreach client
        for (unsigned cc = 0; cc < solution->toRoute.size(); ++cc) {
            Client *client = static_cast<Client *>(solution->toRoute[cc]);
            //foreach tour
            for (unsigned r = 0; r < static_cast<Solution *>(solution)->getNbTour(); ++r) {
                //foreach position
                for (unsigned i = 0;
                     i <= int(static_cast<Tour *>(static_cast<Solution *>(solution)->getTour(r))->getNbClient()); ++i) {

                    bool possible = true;


                    CVRPTW::InsertionCost* cost = static_cast<CVRPTW::InsertionCost*>(static_cast<Tour *>(static_cast<Solution *>(solution)->getTour(
                            r))->evaluateCompletion(client, i,fp));


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

            solution->getTour(best_t)->addClient(static_cast<Solution*>(solution)->notserved[best_client_i], best_p );
            solution->traveltime += bestCost->getDelta();
            solution->toRoute.erase(static_cast<Solution*>(solution)->toRoute.begin() + best_client_i);
            solution->updateTimeVariables(static_cast<Solution*>(solution)->getTour(best_t),best_p,bestCost->getShift());
        }

    }  //end while
    return insertion_found;
}


