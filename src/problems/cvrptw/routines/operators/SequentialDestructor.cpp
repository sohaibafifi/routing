//
// Created by ali on 5/5/19.
//

#include "SequentialDestructor.hpp"
#include "../../models/Tour.hpp"
#include "../../models/Client.hpp"
#include "../../models/Solution.hpp"


void CVRPTW::SequentialDestructor::destruct(routing::models::Solution *solution,routing::DestructionParameters *param)
{
    routing::SequentialDestructionParameters* parameters = static_cast<routing::SequentialDestructionParameters*>(param);
    unsigned long position = parameters->getStartPosition();
    unsigned long length = parameters->getLength();

    if(solution->notserved.size() == solution->getProblem()->clients.size()) return;

    unsigned int nbToursToProcess = solution->getNbTour();

    for(auto i = 0; i < nbToursToProcess ; i++){
        CVRPTW::Tour* tour = static_cast<CVRPTW::Tour*>(solution->getTour(i));

        if(tour->getNbClient() == 0) continue;
        if(tour->getNbClient() <= length){
            for(auto j = 0 ; j < tour->getNbClient(); j++){
                Client * client = static_cast<Client*>(tour->getClient(j));
                static_cast<Solution*>(solution)->notserved.push_back(client);
                routing::Duration traveltime = tour->traveltime;
                tour->removeClient(j);
                static_cast<Solution*>(solution)->traveltime = static_cast<Solution*>(solution)->traveltime
                                                               - traveltime
                                                               + tour->traveltime;

            }

            //static_cast<Solution*>(solution)->removeTour(i);

        }else{
            unsigned long j = 0;

            while( j < length){
                if(tour->getNbClient() == position){
                    Client * client = static_cast<Client*>(tour->getClient(0));
                    static_cast<Solution*>(solution)->notserved.push_back(client);
                    routing::Duration traveltime = tour->traveltime;
                    tour->removeClient(0);
                    static_cast<Solution*>(solution)->traveltime = static_cast<Solution*>(solution)->traveltime
                                                                   - traveltime
                                                                   + tour->traveltime;

                }
                else{
                    Client * client = static_cast<Client*>(tour->getClient(position));
                    static_cast<Solution*>(solution)->notserved.push_back(client);
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

}