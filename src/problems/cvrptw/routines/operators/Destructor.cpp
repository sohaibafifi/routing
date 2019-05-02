//
// Created by ali on 3/28/19.
//

#include "Destructor.hpp"
#include "../../../../mtrand.hpp"

void CVRPTW::Destructor::destruct(routing::models::Solution *solution, unsigned long n, DestructionPolicy policy)
{

    switch (policy){
        case RANDOM:{
            randomDestructoion(solution,n);
            break;
        }

        case SEQUENTIAL:{
            Utilities::MTRand_int32 irand(std::time(nullptr));
            unsigned long position = irand() % n;
            unsigned long length = 1 + irand() % n;
            sequentialDestruction(solution,position,length);
            break;
        }

        default:{
            randomDestructoion(solution,n);
            break;
        }

    }

}

void CVRPTW::Destructor::sequentialDestruction(routing::models::Solution* solution, unsigned long position, unsigned long length){
    if(solution->notserved.size() == solution->getProblem()->clients.size()) return;

    unsigned int nbToursToProcess = solution->getNbTour();

    for(auto i = 0; i < nbToursToProcess ; i++){
        CVRPTW::Tour* tour = static_cast<CVRPTW::Tour*>(solution->getTour(i));


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
            delete tour;
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

void CVRPTW::Destructor::randomDestructoion(routing::models::Solution *solution, unsigned long n)
{
    if(solution->notserved.size() == solution->getProblem()->clients.size()) return;
    Utilities::MTRand_int32 irand(std::time(nullptr));
    unsigned drem = 1 + irand() % n;
    do {

        unsigned t = irand() % solution->getNbTour();
        while ( static_cast<Tour*>(solution->getTour(t))->getNbClient() < 1) {
            t = irand() % solution->getNbTour();
        }
        unsigned long position = (irand() % static_cast<Tour*>(solution->getTour(t))->getNbClient());


        Client * client = static_cast<Client*>(solution->getTour(t)->getClient(position));
        static_cast<Solution*>(solution)->notserved.push_back(client);
        routing::Duration traveltime = static_cast<Tour*>(solution->getTour(t))->traveltime;
        static_cast<Tour*>(solution->getTour(t))->removeClient(position);
        static_cast<Solution*>(solution)->traveltime = static_cast<Solution*>(solution)->traveltime
                                                       - traveltime
                                                       + static_cast<Tour*>(solution->getTour(t))->traveltime;
        solution->update();
    }
    while (static_cast<Solution*>(solution)->notserved.size() < drem);

}
