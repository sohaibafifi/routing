// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Reader.hpp"

#include "models/Client.hpp"
#include "models/Depot.hpp"
#include "models/Vehicle.hpp"
#include <cmath>
#include <queue>
#include <random>
#include <fstream>
#include "Problem.hpp"

routing::Problem *cvrptw::Reader::readFile(std::string filepath) {
    cvrptw::Problem *problem = new cvrptw::Problem();
    std::fstream fh(filepath.c_str(), std::ios_base::in);
    try {

        if (!fh.good()) throw std::string("file open error");
        unsigned nbClients = 100; // FIXME : retrieve the information
        std::string line;
        getline(fh, line);
        problem->name = line;
        problem->name.erase(problem->name.find_last_not_of('\r') + 1);
        getline(fh, line);
        getline(fh, line);
        getline(fh, line);
        getline(fh, line);
        unsigned k;
        routing::Capacity Q;
        std::stringstream(line) >> k >> Q;
        for (unsigned i = 0; i < k; ++i) {
            problem->vehicles.push_back(new cvrptw::models::Vehicle(i, Q));
        }
        getline(fh, line);
        getline(fh, line);
        getline(fh, line);
        getline(fh, line);
        std::vector<routing::Duration> x(nbClients), y(nbClients);
        while (getline(fh, line)) {
            unsigned i;
            routing::Duration _x, _y, service;
            double twopen, twclose;
            routing::Demand demand;
            std::stringstream(line) >> i >> _x >> _y >> demand >> twopen >> twclose >> service;
            routing::TW tw;
            tw.first = twopen;
            tw.second = twclose;

            if (i == 0)
                problem->setDepot(new models::Depot(1, _x, _y, tw));
            else {
                problem->clients.push_back(new models::Client(i, _x, _y, demand, service, tw));
                x[i - 1] = _x;
                y[i - 1] = _y;
            }
        }
        nbClients = problem->clients.size();
        problem->distances = std::vector<std::vector<routing::Duration> >(nbClients);
        problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
        problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients);
        for (unsigned i = 0; i < nbClients; ++i) {
            problem->distances[i] = std::vector<routing::Duration>(nbClients);
            problem->distances_to_depots[0][i] = std::floor( std::hypot(x[i] - problem->getDepot()->getX(),
                                                            y[i] - problem->getDepot()->getY()) * 100) / 100.0;
            for (unsigned j = 0; j < nbClients; ++j)
                problem->distances[i][j] =   std::floor(std::hypot(x[i] - x[j], y[i] - y[j]) * 100) / 100.0;
        }
        return problem;


    }
    catch (std::string emsg) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " :" + emsg);
    }

}
