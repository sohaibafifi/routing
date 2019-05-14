//
// Created by ali on 5/10/19.
//

#include "Diver.hpp"
#include "../../models/Solution.hpp"

#include <ilcplex/ilocplexi.h>
#include "../../../../Utility.hpp"
routing::models::Solution* CVRPTW::Diver::extractPartialSolution(routing::Problem* problem)
{
    CVRPTW::Solution* solution = new Solution(static_cast<CVRPTW::Problem*>(problem));

    IloCplex cplex(problem->model);

    unsigned t = 0, last = 0;
    Tour * tour = new Tour(static_cast<CVRPTW::Problem*>(problem), t, solution->visits);
    do {
        unsigned foundtime = 0;
        for (unsigned j = 0; j < problem->arcs.size(); ++j) {
            if (std::abs(cplex.getValue(problem->arcs[last][j])  - 1 ) <= Configuration::epsilon ) {
                if (last == 0 && foundtime < t) {
                    foundtime++;
                    continue;
                }
                if (j != 0) {
                    solution->notserved.erase(std::remove(solution->notserved.begin(), solution->notserved.end(), static_cast<CVRPTW::Client *>(problem->clients[j - 1])),
                                              solution->notserved.end());
                    tour->pushClient(static_cast<CVRPTW::Client *>(problem->clients[j - 1]));
                }
                last = j;
                break;
            }
        }
        if (last == 0) {
            t++;
            solution->pushTour(tour);
            tour = new Tour(static_cast<CVRPTW::Problem*>(problem), t, solution->visits);
        }

    }
    while (t < problem->vehicles.size() && solution->notserved.size() > 0);

    solution->pushTour(tour);
    if(t == problem->vehicles.size()) delete tour;

    //build empty tours for each unused vechicle
    while(solution->getNbTour() < problem->vehicles.size()) {
        solution->pushTour(new Tour(static_cast<CVRPTW::Problem*>(problem), solution->getNbTour(),solution->visits));
    }

}

routing::forbiddenPositions* CVRPTW::Diver::extractForbiddenPositions(routing::Problem* problem) {
    routing::forbiddenPositions fp;

    for(auto i = 0 ; i < problem->clients.size(); ++i){
        fp.insert(std::make_pair(problem->clients[i]->getID(),std::vector<unsigned int>()));
    }

    IloCplex cplex(problem->model);

    for(unsigned i = 0 ; i < problem->arcs.size(); ++i){
        for (unsigned j = 0; j < problem->arcs.size(); ++j) {
            if (i == j ) continue;

            if (  std::abs(cplex.getValue(problem->arcs[i][j]) - 1) <= Configuration::epsilon ||
                  std::abs(cplex.getValue(problem->arcs[i][j])) <= Configuration::epsilon
               )
            {
                fp[problem->clients[i]->getID()].push_back(problem->clients[j]->getID());
            }

        }
    }

    return &fp;
}


bool CVRPTW::Diver::dive(routing::Problem* problem)
{
    routing::forbiddenPositions* fp = extractForbiddenPositions(problem);
    CVRPTW::Solution* solution = static_cast<CVRPTW::Solution*>(extractPartialSolution(problem));


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
            solution->notserved.erase(static_cast<Solution*>(solution)->notserved.begin() + best_client_i);
            solution->updateTimeVariables(static_cast<Solution*>(solution)->getTour(best_t),best_p,bestCost->getShift());
        }

    }  //end while
    return insertion_found;
}


