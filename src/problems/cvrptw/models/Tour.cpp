//
// Created by ali on 3/28/19.
//

#include "Tour.h"


std::pair<routing::Duration, double> CVRPTW::Tour::evaluateInsertion(routing::models::Client *client, unsigned long position, bool & possible, std::vector<Visit*> visits)
{
    double tij, tic, tcj;;
    double arrival_c, openWindow_c, closeWindow_c;
    double shift_c;
    //double shift_d;
    double wait_j,maxshift_j;


    // check if insertion is possible in term of capacity of vehicle and get the insertion cost

    routing::Duration delta = CVRP::Tour::evaluateInsertion(client, position,possible);

    openWindow_c = static_cast<CVRPTW::Client*>(clients[position])->getEST();
    closeWindow_c = static_cast<CVRPTW::Client*>(clients[position])->getLST();


    if(possible){
            if(position == 0){
               tic = problem->getDistance(*client, *static_cast<CVRPTW::Problem*>(problem)->getDepot());
               arrival_c = tic;
            } else {
                tic = problem->getDistance(*clients[position-1],*client);
                arrival_c = visits[clients[position-1]->getID()]->getStart() + visits[clients[position-1]->getID()]->client->getService() + tic;
            }

            if(arrival_c > closeWindow_c){
                possible = false;
                return std::pair<routing::Duration , double>(std::numeric_limits<routing::Duration>::max(),std::numeric_limits<double>::max());
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

            shift_c = tic + tcj - tij + std::max(0.0,openWindow_c - arrival_c) + static_cast<CVRPTW::Client*>(clients[position])->getService();
            //shift_d = tic + tcj - tij;


            wait_j = visits[clients[position]->getID()]->getWait();
            maxshift_j = visits[clients[position]->getID()]->getMaxshift();

            if(shift_c <= wait_j + maxshift_j){
                possible = true;
                return std::pair<routing::Duration , double>(delta,shift_c);
            }
            else
            {
                possible = false;
                return std::pair<routing::Duration , double>(std::numeric_limits<routing::Duration>::max(),std::numeric_limits<double>::max());
            }


    }

}


