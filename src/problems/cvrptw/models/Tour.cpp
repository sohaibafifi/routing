//
// Created by ali on 3/28/19.
//

#include "Tour.hpp"
#include "Solution.hpp"


routing::InsertionCost* CVRPTW::Tour::evaluateInsertion(routing::models::Client *client, unsigned long position, std::vector<Visit*> visits)
{
    double tij, tic, tcj;;
    double arrival_c, openWindow_c, closeWindow_c;
    double shift_c;
    //double shift_d;
    double wait_j,maxshift_j;


    // check if insertion is possible in term of capacity of vehicle and get the insertion cost

    CVRP::InsertionCost* cost = static_cast<CVRP::InsertionCost*> (CVRP::Tour::evaluateInsertion(client, position));


    openWindow_c = static_cast<CVRPTW::Client*>(client)->getEST();
    closeWindow_c = static_cast<CVRPTW::Client*>(client)->getLST();


    if(cost->isPossible()){
            InsertionCost* cost_tw = new InsertionCost(cost->getDelta(),cost->isPossible(),0);
            if(position == 0){
               tic = problem->getDistance(*client, *static_cast<CVRPTW::Problem*>(problem)->getDepot());
               arrival_c = tic;

            } else {
                tic = problem->getDistance(*clients[position-1],*client);
                arrival_c = visits[clients[position-1]->getID()]->getStart() + visits[clients[position-1]->getID()]->client->getService() + tic;
            }

            if(arrival_c > closeWindow_c){
                cost_tw->setPossible(false);
                cost_tw->setDelta(std::numeric_limits<routing::Duration>::max());
                cost_tw->setShift(std::numeric_limits<routing::Duration>::max());
                return cost_tw;
            }

            if(position == clients.size()){ //check if insertion is at the end
                //vehicle returns to depot, so distance between last client and depot
                tcj = problem->getDistance(*client, *static_cast<CVRPTW::Problem*>(problem)->getDepot());
            }
            else {
                //distance between client to insert and actual client at position
                tcj = problem->getDistance(*client, *clients[position] );
            }

            //distance between clients at position - 1 and position before insertion
            if (position == 0 &&  position == clients.size()){ // if insertion is at first and there is no client in the tour
                tij = 0.0;
            }
            else{
                if(position == 0){
                    tij = problem->getDistance(*clients[position], *static_cast<CVRPTW::Problem*>(problem)->getDepot());

                } else if (position == clients.size()){
                    tij = problem->getDistance(*clients[position-1], *static_cast<CVRPTW::Problem*>(problem)->getDepot());
                } else{
                    tij = problem->getDistance(*clients[position-1],*clients[position]);
                }
            }

            shift_c = tic + tcj - tij + std::max(0.0,openWindow_c - arrival_c) + static_cast<CVRPTW::Client*>(client)->getService();
            //shift_d = tic + tcj - tij;

            if(position == clients.size()){
                wait_j = 0 ;
                //maxshift_j = std::max(static_cast<CVRPTW::Client*>(client)->getLST() - visits[client->getID()]->getStart() , 0.0);
                 maxshift_j = static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getLST() -
                       static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getEST();

            }else{
                wait_j = visits[clients[position]->getID()]->getWait();
                maxshift_j = visits[clients[position]->getID()]->getMaxshift();
            }

            if(shift_c <= wait_j + maxshift_j){
                cost_tw->setPossible(true);
                cost_tw->setDelta(cost->getDelta());
                cost_tw->setShift(shift_c);
                return cost_tw;
            }
            else
            {
                cost_tw->setPossible(false);
                cost_tw->setDelta(std::numeric_limits<routing::Duration>::max());
                cost_tw->setShift(std::numeric_limits<routing::Duration>::max());
                return cost_tw;
            }


    }

}

routing::Duration CVRPTW::Tour::evaluateRemove(unsigned long position){
    return CVRP::Tour::evaluateRemove(position);
}


void CVRPTW::Tour::removeClient(unsigned long position) {
    CVRP::Tour::removeClient(position);
}


void CVRPTW::Tour::addClient(routing::models::Client *client, unsigned long position)
{
    CVRP::Tour::addClient(client, position);
    // update the visit and the tour

}
