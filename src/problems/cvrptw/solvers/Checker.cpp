//
// Created by ali on 4/15/19.
//

#include "Checker.hpp"

bool CVRPTW::Checker::check()
{

    //CVRP::Checker* checker = new CVRP::Checker(this->getSolution(),out);
    //bool cvrp_check = checker->check();

    CVRPTW::Solution* sol = static_cast<CVRPTW::Solution*>(solution);

    if (!sol->notserved.empty()) {
        this->out << "Error! Not all client have been routed \n";
        return false;
    }

    std::vector<bool> clients(sol->getProblem()->clients.size(), false);
    for (auto i = 0; i < sol->getNbTour(); i++) {
        if (static_cast<CVRP::Tour *>(sol->getTour(i))->consumption >
            static_cast<CVRP::Vehicle *>(sol->getProblem()->vehicles[sol->getTour(i)->getID()])->getCapacity()) {
            this->out << "Error! Tour " << i << " exceeds vehicle capacity\n";
            return false;
        }

        for (auto j = 0; j < static_cast<CVRP::Tour *>(sol->getTour(i))->getNbClient(); j++) {

            if (clients[static_cast<CVRP::Tour *>(sol->getTour(i))->getClient(j)->getID()] == true) {
                std::cout << "Error : client " << j << " already visited\n";
                return false;


            } else {
                clients[static_cast<CVRP::Tour *>(sol->getTour(i))->getClient(j)->getID()] = true;
            }

        }

    }

    routing::Duration start = 0.0;
    //if(cvrp_check){
        for(auto i = 0; i < sol->getNbTour(); i++){
            CVRPTW::Tour* tour = static_cast<CVRPTW::Tour*>( sol->getTour(i) );
            for(auto j=0; j < static_cast<CVRPTW::Tour*>( sol->getTour(i) )->getNbClient(); j++){
                CVRPTW::Client* client = static_cast<CVRPTW::Client*>(tour->getClient(j));
                if (tour->visits[client->getID()]->getStart() < client->getEST()  ){
                    out << "Error in tour: "<<  i << " for client " << client->getID() <<" start date is before time window start\n";
                    return false;
                }
                if(tour->visits[client->getID()]->getStart() > client->getLST()){
                    out << "Error in tour: "<<  i << " for client " << client->getID() << " start date is after time window end\n";
                    return false;
                }

            }

        }

        return true;
    //}
    //else{
    //    return false;
    //}
}