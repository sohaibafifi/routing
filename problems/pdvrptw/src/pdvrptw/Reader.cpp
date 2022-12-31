//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#include "Reader.hpp"

#include "models/Client.hpp"
#include "Problem.hpp"
#include <cmath>
#include <queue>
#include <fstream>

routing::Problem *pdvrptw::Reader::readFile(const std::string & filePath) {
    try {
        auto *problem = new pdvrptw::Problem(*dynamic_cast<cvrptw::Problem *>(cvrptw::Reader::readFile(filePath)));

        // convert the cvp clients to pdvrptw clients
        for (auto &client: problem->clients) {
            client = new pdvrptw::models::Client(
                    dynamic_cast<cvrptw::models::Client *>(client)
            );

        }

        return problem;
    }
    catch (std::string emsg) {
        throw std::string(std::string("pdvrptw::Reader::readFile : ") + emsg);
    }

}
