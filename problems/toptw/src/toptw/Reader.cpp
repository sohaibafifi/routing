//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#include "Reader.hpp"

#include "models/Client.hpp"
#include "Problem.hpp"
#include <cmath>
#include <queue>
#include <fstream>

routing::Problem *toptw::Reader::readFile(std::string filePath) {
    try {
        auto *problem = new toptw::Problem(*dynamic_cast<cvrptw::Problem *>(cvrptw::Reader::readFile(filePath)));

        // convert the cvp clients to toptw clients
        for (auto &client: problem->clients) {
            client = new toptw::models::Client(
                    dynamic_cast<cvrptw::models::Client *>(client)
            );

        }

        while (problem->vehicles.size() > problem->clients.size() / 10) {
            problem->vehicles.pop_back();
         }

        return problem;
    }
    catch (std::string emsg) {
        throw std::string(std::string("toptw::Reader::readFile : ") + emsg);
    }

}
