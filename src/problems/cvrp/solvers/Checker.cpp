//
// Created by ali on 4/15/19.
//

#include "Checker.hpp"
#include "../models/Solution.hpp"

bool CVRP::Checker::check() {

    CVRP::Solution *sol = static_cast<CVRP::Solution *>(this->getSolution());

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
    return true;
}