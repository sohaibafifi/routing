#include "Tour.hpp"

routing::InsertionCost* CVRP::Tour::evaluateInsertion(routing::models::Client *client, unsigned long position)
{
    InsertionCost* cost = new InsertionCost(0,true);

    if (this->consumption + static_cast<CVRP::Client*>(client)->getDemand() >
            static_cast<CVRP::Vehicle *>(problem->vehicles[this->getID()])->getCapacity())
    {
        cost->setPossible(false);
        cost->setDelta(std::numeric_limits<routing::Duration>::max());
        return cost;
    }
    routing::Duration delta = 0;
    if(clients.empty())
    {
        delta = 2 * problem->getDistance(*client, *static_cast<CVRP::Problem*>(problem)->getDepot());
    }
    else {
        if(position == 0){
            delta = problem->getDistance(*client, *clients[position])
                    + problem->getDistance(*client, *static_cast<CVRP::Problem*>(problem)->getDepot())
                    - problem->getDistance(*clients[position], *static_cast<CVRP::Problem*>(problem)->getDepot());
        }else {
            if(position == clients.size()){
                delta = problem->getDistance(*clients[position - 1], *client)
                        + problem->getDistance(*client, *static_cast<CVRP::Problem*>(problem)->getDepot())
                        - problem->getDistance(*clients[position - 1], *static_cast<CVRP::Problem*>(problem)->getDepot());

            }else {
                delta = problem->getDistance(*clients[position - 1], *client)
                        + problem->getDistance(*client, *clients[position])
                        - problem->getDistance(*clients[position - 1], *clients[position]);
            }
        }
    }
    cost->setPossible(true);
    cost->setDelta(delta);
    return cost;
}

routing::RemoveCost* CVRP::Tour::evaluateRemove(unsigned long position)
{
    Client * client = clients[position];
    routing::Duration delta = 0;
    if (position == 0) {
        if (position == clients.size() - 1) {
            delta = 2 * problem->getDistance(*client, *static_cast<CVRP::Problem*>(problem)->getDepot());
        }
        else {
            delta = - problem->getDistance(*client, *static_cast<CVRP::Problem*>(problem)->getDepot())
                    - problem->getDistance(*client, *clients[position + 1])
                    + problem->getDistance(*clients[position + 1], *static_cast<CVRP::Problem*>(problem)->getDepot());
        }
    }
    else {
        if (position == clients.size() - 1) {
            delta = - problem->getDistance(*clients[position - 1], *client)
                    - problem->getDistance(*client, *static_cast<CVRP::Problem*>(problem)->getDepot())
                    + problem->getDistance(*clients[position - 1], *static_cast<CVRP::Problem*>(problem)->getDepot());

        }
        else {
            delta = - problem->getDistance(*clients[position - 1], *client)
                    - problem->getDistance(*client, *clients[position + 1])
                    + problem->getDistance(*clients[position - 1], *clients[position + 1]);
        }
    }
    return new RemoveCost(delta);
}

void CVRP::Tour::removeClient(unsigned long position)
{
    consumption -= clients[position]->getDemand();
    traveltime += static_cast<CVRP::RemoveCost*>(this->evaluateRemove(position))->getDelta();
    clients.erase(clients.begin() + position);
}

routing::models::Client *CVRP::Tour::getClient(unsigned long c) const
{
    return clients[c];
}

unsigned long CVRP::Tour::getNbClient()
{
    return clients.size();
}


void CVRP::Tour::pushClient(routing::models::Client *client)
{
    traveltime += static_cast<CVRP::InsertionCost*>(CVRP::Tour::evaluateInsertion(client, getNbClient()))->getDelta();
    consumption += static_cast<Client*>(client)->getDemand();
    clients.insert(clients.begin() + getNbClient(), static_cast<Client*>(client));
}

void CVRP::Tour::addClient(routing::models::Client *client, unsigned long position)
{
    bool possible = true; // we assume that this is already verified
    //TODO : optimize to not recalculate the insertion cost
    traveltime += static_cast<InsertionCost*>(this->evaluateInsertion(client, position))->getDelta();
    consumption += static_cast<Client*>(client)->getDemand();
    clients.insert(clients.begin() + position, static_cast<Client*>(client));
}

