//
// Created by ali on 3/28/19.
//

#include "Solution.h"



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

void CVRPTW::Solution::updateMaxShiftAtPosition(routing::models::Tour *tour, unsigned int position, double shift_j) {
    Visit* visit = visits[static_cast<Tour*>(tour)->getClient(position)->getID()];
    routing::Duration wait = std::max(0.0, visit->getWait()-shift_j);
}


void CVRPTW::Solution::updateWaitAtPosition(routing::models::Tour *tour, unsigned int position, double shift_j) {
    Visit* visit = visits[static_cast<Tour*>(tour)->getClient(position)->getID()];
    routing::Duration wait = std::max(0.0, visit->getWait()-shift_j);
    visit->setWait(wait);
}



void CVRPTW::Solution::updateTimeVariables(routing::models::Tour *tour, unsigned int position, double shift_j)
{
    double shift_k = shift_j;
    unsigned int j = position;
    while(j <= static_cast<Tour*>(tour)->getNbClient() && shift_k != 0  ){
        Visit* visit = visits[static_cast<Tour*>(tour)->getClient(j)->getID()];
        routing::Duration wait_k = visit->getWait();
        //routing::Duration maxshift_k = visit->getMaxshift();

        visit->setWait(std::max(0.0, wait_k - shift_k));
        shift_k = std::max(0.0,shift_j - wait_k);
        visit->setStart(visit->getStart()+shift_k);
        visit->setMaxshift(visit->getMaxshift()-shift_k);



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



void CVRPTW::Tour::addClient(routing::models::Client *client, unsigned long position)
{
    CVRP::Tour::addClient(client, position);
    // update the visit and the tour

}


void CVRPTW::Solution::addTour(routing::models::Tour *tour, unsigned long position)
{
    tours.insert(tours.begin() + position, static_cast<Tour*>(tour));
    traveltime += static_cast<Tour*>( tour )->traveltime;
}

routing::models::Solution *CVRPTW::Solution::clone() const {
    return new Solution(*this);
}


