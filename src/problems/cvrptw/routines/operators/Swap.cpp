//
// Created by ali on 6/18/19.
//

#include "Swap.hpp"
#include "../../models/Solution.hpp"

bool CVRPTW::Swap::look(routing::models::Solution *solution) {
    return routing::Swap::look(solution);
}


bool CVRPTW::Swap::doSwap(routing::models::Solution *solution, std::pair<int, int> tourPosition)
{

    CVRPTW::Solution* sol = static_cast<CVRPTW::Solution*>(solution);

    //backup before removal, if no changes can be operated to solution use this backup before leaving
    //CVRPTW::Solution* backupSolution_ = static_cast<CVRPTW::Solution*>(sol->clone());

    CVRPTW::Tour* bTour = new Tour(*static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first)));
    CVRPTW::Client* client = static_cast<CVRPTW::Client*>(static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->getClient(tourPosition.second));

    //update traveltime after client removal
    routing::Duration traveltime = static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->traveltime;
    static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->removeClient(tourPosition.second);
    sol->traveltime = sol->traveltime - traveltime + static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->traveltime;

    //keep backup after client removal
    //CVRPTW::Solution* backupSolution = static_cast<CVRPTW::Solution*>(sol->clone());

    //for each tour
    for (int i = 0; i < sol->getNbTour(); ++i) {
        //get actual tour and back it up
        CVRPTW::Tour* tour = static_cast<CVRPTW::Tour*>(sol->getTour(i));
        CVRPTW::Tour* backupTour = new Tour(*tour);

        //for each position
        for (int j = 0; j < tour->getNbClient(); ++j) {

            //do not insert at original position
            if(i == tourPosition.first && j == tourPosition.second) continue;

            // remove client with whom to swap and update traveltime
            routing::Duration removedClientTraveltime = tour->traveltime;
            CVRPTW::Client* clientToSwap = static_cast<CVRPTW::Client*>(tour->getClient(j));
            tour->removeClient(j);
            sol->traveltime = sol->traveltime - removedClientTraveltime + tour->traveltime;

            CVRPTW::InsertionCost* cost = static_cast<CVRPTW::InsertionCost*>(tour->evaluateInsertion(client, j));

            if(cost->isPossible()){
                tour->addClient(client, j );
                sol->updateTimeVariables(tour,j,cost->getShift());
                sol->traveltime += cost->getDelta();

                CVRPTW::InsertionCost* costSwap = static_cast<CVRPTW::InsertionCost*>((sol->getTour(tourPosition.first))->evaluateInsertion(clientToSwap, tourPosition.second));

                if(costSwap->isPossible()){
                    sol->getTour(tourPosition.first)->addClient(clientToSwap,tourPosition.second);
                    sol->traveltime += costSwap->getDelta();
                    sol->updateTimeVariables(sol->getTour(tourPosition.first),tourPosition.second,costSwap->getShift());
                    return true;
                }
                else
                {
                    //sol = static_cast<CVRPTW::Solution*>(backupSolution->clone());
                    sol->removeTour(i);
                    sol->traveltime -= tour->traveltime;
                    tour = new Tour(*backupTour);
                    sol->addTour(tour,i);
                }
            }else{
                //sol = static_cast<CVRPTW::Solution*>(backupSolution->clone());
                sol->removeTour(i);
                sol->traveltime -= tour->traveltime;
                tour = new Tour(*backupTour);
                sol->addTour(tour,i);
            }


        }
    }
    sol->traveltime -= static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->traveltime;
    sol->removeTour(tourPosition.first);
    sol->addTour(new Tour(*bTour),tourPosition.first);
    //sol = new Solution(*backupSolution_);

    return false;


}