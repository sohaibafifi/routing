//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#include "Reader.hpp"

#include "models/Client.hpp"
#include "Problem.hpp"
#include <cmath>
#include <queue>
#include <fstream>

routing::Problem *top::Reader::readFile(const std::string & filepath) {
    try {
        auto *problem = new top::Problem(*dynamic_cast<cvrp::Problem *>(cvrp::Reader::readFile(filepath)));

        // convert the cvp clients to top clients
        for (auto &client: problem->clients) {
            client = new top::models::Client(
                    dynamic_cast<cvrp::models::Client *>(client)
            );

        }

        // while (problem->vehicles.size() > problem->clients.size() / 10) {
        //    problem->vehicles.pop_back();
        // }

        return problem;
    }
    catch (std::string emsg) {
        throw std::string(std::string("top::Reader::readFile : ") + emsg);
    }

}
