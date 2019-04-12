//
// Created by ali on 3/28/19.
//

#include "Tour.hpp"
#include "Solution.hpp"


routing::InsertionCost* CVRPTW::Tour::evaluateInsertion(routing::models::Client *client, unsigned long position)
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
                //maxshift_j = std::min(static_cast<CVRPTW::Client*>(client)->getLST() - visits[client->getID()]->getStart() , static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getLST() - static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getEST());
                maxshift_j = static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getLST() - static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getEST();

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
    updateTimeVariablesAfterRemove(position);
}

void CVRPTW::Tour::updateTimeVariablesAfterRemove(unsigned long position){
    routing::Duration arrival = 0.0;
    routing::Duration tic = 0.0;
    routing::Duration openWindow = 0.0, closeWindow = 0.0 ;
    routing::Duration wait = 0.0, wait_j;
    routing::Duration maxshift = 0.0 , maxshift_j = 0.0 ;
    routing::Duration start = 0.0;
    if(clients.empty()) return;
    for(unsigned i = position ; i < clients.size(); i++) {
        //compute arrival
        if (i == 0) {
            arrival = problem->getDistance(*clients[i], *static_cast<CVRPTW::Problem *>(problem)->getDepot());

        } else {
            tic = problem->getDistance(*clients[i - 1], *clients[i]);
            arrival = visits[clients[i - 1]->getID()]->getStart() +
                      visits[clients[i - 1]->getID()]->client->getService() + tic;
        }
        //get opening time window
        openWindow = static_cast<CVRPTW::Client *>(clients[i])->getEST();
        //compute wait and set it
        wait = std::max(0.0, openWindow - arrival);
        visits[clients[i]->getID()]->setWait(wait);

        //update start
        start = std::max(openWindow, arrival);
        visits[clients[i]->getID()]->setStart(start);
    }
    int length = clients.size();
    for(auto i = length - 1 ;  i >= 0 ; i--){
        //get maxshift for last position as depot's maxshift
        if(i == clients.size() - 1 ){
            maxshift_j = static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getLST() -
                         static_cast<CVRPTW::Depot*>(static_cast<CVRPTW::Problem*>(problem)->getDepot())->getEST();
            wait_j = 0.0;

        }else{ // get maxshift of next client
            maxshift_j = visits[clients[i+1]->getID()]->getMaxshift();
            wait_j = visits[clients[i+1]->getID()]->getWait();
        }

        closeWindow = static_cast<CVRPTW::Client*>(clients[i])->getLST();
        start = visits[clients[i]->getID()]->getStart();

        maxshift = std::min( closeWindow - start, wait_j + maxshift_j );

        visits[clients[i]->getID()]->setMaxshift(maxshift);

    }
}

void CVRPTW::Tour::addClient(routing::models::Client *client, unsigned long position)
{
    CVRP::Tour::addClient(client, position);
    // update the visit and the tour

}
