//
// Created by ali on 6/26/19.
//

#include "Destructor_TSPTWH.hpp"
#include "../../../../routines/operators/TSPTWDestructionParameters.hpp"
#include "../../models/Solution.hpp"


void CVRPTW::Destructor_TSPTWH::destruct(routing::models::Solution *solution, routing::DestructionParameters* param)
{
    routing::TSPTWDestructionParameters* parameters = static_cast<routing::TSPTWDestructionParameters*>(param);
    unsigned long position = parameters->getStartPosition();
    unsigned long length = 0;


    if(solution->notserved.size() == solution->getProblem()->clients.size()) return;

    //get the tour to process
    CVRPTW::Tour* tour = static_cast<CVRPTW::Tour*>(solution->getTour(parameters->getTourIndex()));


    if(tour->getNbClient() == 0) return;

    //get endPosition circularly
    unsigned long endPosition = parameters->getEndPosition() % tour->getNbClient();

    //compute length with respect to circularity
    if(endPosition < position )
    {
        length = tour->getNbClient() - position + endPosition + 1;
    }else
    {
        length = endPosition - position + 1;
    }

    // check if we have to delete all clients in the tour
    // case that number of clients to destroy is greater or equal to number of clients in the tour
    if(tour->getNbClient() <= length){
        for(auto j = 0 ; j < tour->getNbClient(); j++){
            Client * client = static_cast<Client*>(tour->getClient(j));
            parameters->addClientToRout(client);
            routing::Duration traveltime = tour->traveltime;
            tour->removeClient(j);
            static_cast<Solution*>(solution)->traveltime = static_cast<Solution*>(solution)->traveltime
                                                           - traveltime
                                                           + tour->traveltime;

        }

    }else{ //number of clients in the tour is grater than number of clients to destroy
        unsigned long j = 0;
        //begin destruction of "length" clients
        while( j < length){
            //if we reached a position that is greater than the last client index in the tour
            //then we destroy circularly from the beginning of the tour
            if(tour->getNbClient() <= position){
                Client * client = static_cast<Client*>(tour->getClient(0));
                parameters->addClientToRout(client);
                routing::Duration traveltime = tour->traveltime;
                tour->removeClient(0);
                static_cast<Solution*>(solution)->traveltime = static_cast<Solution*>(solution)->traveltime
                                                               - traveltime
                                                               + tour->traveltime;

            }
            else{
                Client * client = static_cast<Client*>(tour->getClient(position));
                parameters->addClientToRout(client);
                routing::Duration traveltime = tour->traveltime;
                tour->removeClient(position);
                static_cast<Solution*>(solution)->traveltime = static_cast<Solution*>(solution)->traveltime
                                                               - traveltime
                                                               + tour->traveltime;

            }
            j++;
        }
    }

}