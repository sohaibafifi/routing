//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#include "Reader.hpp"

#include "models/Client.hpp"
#include "Problem.hpp"
#include <cmath>
#include <queue>
#include <fstream>

routing::Problem *pdvrp::Reader::readFile(std::string filePath) {
    try {
        auto *problem = new pdvrp::Problem(*dynamic_cast<cvrp::Problem *>(cvrp::Reader::readFile(filePath)));

        // convert the cvp clients to pdvrp clients
        for (auto &client: problem->clients) {
            client = new pdvrp::models::Client(
                    dynamic_cast<cvrp::models::Client *>(client)
            );

        }

        return problem;
    }
    catch (std::string emsg) {
        throw std::string(std::string("pdvrp::Reader::readFile : ") + emsg);
    }

}
