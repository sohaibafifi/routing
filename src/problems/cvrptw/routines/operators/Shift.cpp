//
// Created by ali on 5/28/19.
//

#include "Shift.hpp"
#include "../../models/Solution.hpp"

bool CVRPTW::Shift::look(routing::models::Solution *solution) {
   return routing::Shift::look(solution);
}

bool CVRPTW::Shift::doShift(routing::models::Solution *solution, std::pair<int, int> tourPosition)
{

    /*std::cout << "From doShift" << std::endl;
    for (int l = 0; l < solution->getNbTour(); ++l) {
        if (solution->getTour(l)->getNbClient() == 0) continue;
        std::cout << "Tour " << l << "  ";
        for (int i = 0; i < solution->getTour(l)->getNbClient(); ++i) {
            std::cout << solution->getTour(l)->getClient(i)->getID() << std::setw(3);
        }
        std::cout << std::endl;
    }*/

    CVRPTW::Solution* sol = static_cast<CVRPTW::Solution*>(solution);
    CVRPTW::Client* client = static_cast<CVRPTW::Client*>(static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->getClient(tourPosition.second));
    //update traveltime after client removal
    routing::Duration traveltime = static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->traveltime;
    static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->removeClient(tourPosition.second);
    sol->traveltime = sol->traveltime - traveltime + static_cast<CVRPTW::Tour*>(sol->getTour(tourPosition.first))->traveltime;

    //for each tour
    for (int i = 0; i < sol->getNbTour(); ++i) {
        CVRPTW::Tour* tour = static_cast<CVRPTW::Tour*>(sol->getTour(i));
        //for each position
        for (int j = 0; j <= tour->getNbClient(); ++j) {
            //do not insert at original position
            if(i == tourPosition.first && j == tourPosition.second) continue;

            CVRPTW::InsertionCost* cost = static_cast<CVRPTW::InsertionCost*>(tour->evaluateInsertion(client, j));

            if(cost->isPossible()){
                tour->addClient(client, j );
                sol->traveltime += cost->getDelta();
                sol->updateTimeVariables(tour,j,cost->getShift());
                return true;
            }


        }
    }

    return false;
}