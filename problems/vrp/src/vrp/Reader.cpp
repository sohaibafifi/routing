//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#include "Reader.hpp"

#include "models/Client.hpp"
#include "models/Depot.hpp"
#include <cmath>
#include <random>
#include "Problem.hpp"

vrp::Problem *vrp::Reader::readFile(std::string filepath) {
    auto *problem = new vrp::Problem();
    problem->setName("dummy");
    problem->vehicles = std::vector<routing::models::Vehicle *>();
    for (unsigned int i = 1; i <= 50; ++i) {
        problem->vehicles.push_back(new routing::models::Vehicle(i));
    }
    problem->depots = std::vector<routing::models::Depot *>();
    problem->setDepot(new vrp::models::Depot(1, 0, 0));
    problem->clients = std::vector<routing::models::Client *>();
    std::random_device rd;
    for (int i = 1; i <= 10; ++i) {
        problem->clients.push_back(new vrp::models::Client(i, rd() % 100, rd() % 100));
    }
    unsigned long nbClients = problem->clients.size();
    problem->distances = std::vector<std::vector<routing::Duration> >(nbClients);
    problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
    problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients);
    for (unsigned i = 0; i < nbClients; ++i) {
        problem->distances[i] = std::vector<routing::Duration>(problem->clients.size());
        problem->distances_to_depots[0][i] =
                round(std::sqrt(
                        (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getX() -
                         problem->getDepot()->getX()) *
                        (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getX() -
                         problem->getDepot()->getX()) +
                        (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getY() -
                         problem->getDepot()->getY()) *
                        (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getY() - problem->getDepot()->getY())

                ));
        for (unsigned j = 0; j < nbClients; ++j)
            problem->distances[i][j] =
                    round(std::sqrt(
                            (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getX() -
                             dynamic_cast<vrp::models::Client *>(problem->clients[j])->getX()) *
                            (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getX() -
                             dynamic_cast<vrp::models::Client *>(problem->clients[j])->getX()) +
                            (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getY() -
                             dynamic_cast<vrp::models::Client *>(problem->clients[j])->getY()) *
                            (dynamic_cast<vrp::models::Client *>(problem->clients[i])->getY() -
                             dynamic_cast<vrp::models::Client *>(problem->clients[j])->getY())

                    ));
    }
    return problem;
}
