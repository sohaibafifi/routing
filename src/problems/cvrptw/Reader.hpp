//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_READER_H
#define HYBRID_READER_H

#include "../cvrp/Reader.hpp"
#include "models/Problem.hpp"
#include "models/Vehicle.hpp"
#include "models/Client.hpp"
#include "models/Depot.hpp"
#include "../../data/attributes/Rendezvous.hpp"
#include "../../Configurations.hpp"
namespace CVRPTW{
    class Reader
            : public routing::Reader
    {
    public:
        virtual CVRPTW::Problem *readFile(std::string filepath);
    };
}

#endif //HYBRID_READER_H
