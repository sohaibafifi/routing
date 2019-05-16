//
// Created by ali on 3/28/19.
//

#include "Reader.hpp"
#include <cmath>

CVRPTW::Problem *CVRPTW::Reader::readFile(std::string filepath)
{
    CVRPTW::Problem *problem = new CVRPTW::Problem();
    std::fstream fh(filepath.c_str(), std::ios_base::in);
    try {

        if (!fh.good()) throw std::string("file open error");
        unsigned nbClients = 100;
        std::string line;
        getline(fh, line);
        problem->name = line;
        getline(fh, line);
        getline(fh, line);
        getline(fh, line);
        getline(fh, line);
        unsigned k;
        routing::Capacity Q;
        std::stringstream(line) >> k >> Q;
        for (unsigned i = 0; i < k; ++i) {
            problem->vehicles.push_back(new Vehicle(i, Q));
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
                problem->setDepot(new Depot(1, _x, _y, tw));
            else {
                problem->clients.push_back(new Client(i, demand, service, tw, _x, _y));
                x[i - 1] = _x;
                y[i - 1] = _y;
            }
        }
        problem->distances = std::vector<std::vector<routing::Duration> >(nbClients);
        problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
        problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients);
        for (unsigned i = 0; i < nbClients; ++i) {
            problem->distances[i] = std::vector<routing::Duration>(nbClients);
            problem->distances_to_depots[0][i] = floor(std::hypot(x[i] - problem->getDepot()->getX(), y[i] - problem->getDepot()->getY())*Configuration::precision)/Configuration::precision;
            for (unsigned j = 0; j < nbClients; ++j)
                problem->distances[i][j] = floor(std::hypot(x[i] - x[j], y[i] - y[j])* Configuration::precision)/Configuration::precision;
        }
        return problem;

    }
    catch (std::string emsg) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " : " + emsg);
    }

}
