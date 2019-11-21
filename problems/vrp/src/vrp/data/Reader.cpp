//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#include "Reader.hpp"


routing::Problem *vrp::Reader::readFile(std::string filepath) {
    vrp::Problem *problem = new vrp::Problem();
    problem->setName("dummy");
    problem->vehicles = std::vector<routing::models::Vehicle *>();
    problem->depots = std::vector<routing::models::Depot *>();
    problem->clients = std::vector<routing::models::Client *>();
    return problem;
}
