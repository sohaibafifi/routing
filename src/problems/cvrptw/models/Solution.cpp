//
// Created by ali on 3/28/19.
//

#include "Solution.hpp"



CVRPTW::Solution::Solution(CVRPTW::Problem *problem):
        CVRP::Solution(problem),
        tours(std::vector<Tour *>()){
    visits = std::vector<Visit*>(problem->clients.size() + 1);
    for (unsigned long i = 0; i < problem->clients.size(); ++i) {
        Client * client = static_cast<Client *>(problem->clients[i]);
        visits[client->getID()] =
                new Visit(client,
                          client->getEST(),
                          client->getLST() - client->getEST(),
                          0.0);
    }
}


void CVRPTW::Solution::updateMaxShifts()
{
    std::cout << "updating maxshift " << std::endl;
}


void CVRPTW::Solution::updateTimeVariables(routing::models::Tour *tour, unsigned int position, double shift_j)
{
    double shift_k = shift_j;
    unsigned int j = position;

    //update wait and start of inserted client

    Client* inserted_client = static_cast<CVRPTW::Client*>(static_cast<CVRPTW::Tour*>(tour)->getClient(position));
    Visit* visit_of_inserted_client = visits[inserted_client->getID()];
    routing::Duration arrival;
    //get arrival
    if (position == 0) {
        arrival = problem->getDistance(*inserted_client, *static_cast<CVRPTW::Problem *>(problem)->getDepot());

    } else {
        arrival = visits[static_cast<CVRPTW::Tour*>(tour)->getClient(position-1)->getID()]->getStart() +
                  visits[static_cast<CVRPTW::Tour*>(tour)->getClient(position-1)->getID()]->client->getService() +
                  problem->getDistance(*static_cast<CVRPTW::Tour*>(tour)->getClient(position-1), *inserted_client);
    }

    routing::Duration wait_inserted_client = std::max(0.0, inserted_client->getEST() - arrival);
    routing::Duration start_inserted_client = std::max( arrival , inserted_client->getEST() );
    visit_of_inserted_client->setWait(wait_inserted_client);
    visit_of_inserted_client->setStart(start_inserted_client);

    // update start, maxshift and wait for clients after position
    for(j=position+1; j< static_cast<Tour*>(tour)->getNbClient(); j++){
        if(shift_k == 0) break;
        Visit* visit = visits[static_cast<CVRPTW::Tour*>(tour)->getClient(j)->getID()];
        routing::Duration wait_k = visit->getWait();
        //routing::Duration maxshift_k = visit->getMaxshift();

        visit->setWait(std::max(0.0, wait_k - shift_k));
        shift_k = std::max(0.0,shift_j - wait_k);
        visit->setStart(visit->getStart()+shift_k);
        visit->setMaxshift(visit->getMaxshift()-shift_k);

    }

    //update maxshift for inserted client
    routing::Duration maxshift_j = 0 , wait_j = 0;

    maxshift_j = static_cast<CVRPTW::Tour*>(tour)->getNextMaxshift(position);
    wait_j = static_cast<CVRPTW::Tour*>(tour)->getNextWait(position);

    routing::Duration closeWindow = static_cast<CVRPTW::Client*>(inserted_client)->getLST();
    routing::Duration start = visit_of_inserted_client->getStart();

    visit_of_inserted_client->setMaxshift(std::min( closeWindow - start, wait_j + maxshift_j ));

    //update clients before position
    j = position;


    while(j>0){

        Client* client_to_update = static_cast<CVRPTW::Client*>(static_cast<CVRPTW::Tour*>(tour)->getClient(j-1));
        Visit* visit_to_update = visits[client_to_update->getID()];

        //take wait and maxshift of client at position = j
        routing::Duration maxshift_for_update = visit_of_inserted_client->getMaxshift();
        routing::Duration wait_for_update = visit_of_inserted_client->getWait();

        //update maxshift of client j-1
        visit_to_update->setMaxshift(std::min(client_to_update->getLST()-visit_to_update->getStart(),wait_for_update+maxshift_for_update));

        //go to next iteration
        visit_of_inserted_client = visit_to_update;
        j--;
    }

}

void CVRPTW::Solution::print(std::ostream &out)
{
    out << "solution cost " << getCost() << std::endl;
    for (unsigned t = 0;t < getNbTour();t++) {
        out << "tour " << t << " : ";
        for (unsigned i = 0; i < tours[t]->getNbClient(); ++i) {
            out << tours[t]->getClient(i)->getID() << "(" << static_cast<Client*>( tours[t]->getClient(i))->getService() << ") ";
        }
        out << std::endl;
    }
}

void CVRPTW::Solution::update()
{

}


routing::models::Tour *CVRPTW::Solution::getTour(unsigned t)
{
    return tours.at(t);
}


void CVRPTW::Solution::addTour(routing::models::Tour *tour, unsigned long position)
{
    tours.insert(tours.begin() + position, static_cast<Tour*>(tour));
    traveltime += static_cast<Tour*>( tour )->traveltime;
}

routing::models::Solution *CVRPTW::Solution::clone() const {
    return new Solution(*this);
}


