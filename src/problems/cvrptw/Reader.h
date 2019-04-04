//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_READER_H
#define HYBRID_READER_H

#include "../cvrp/Reader.hpp"
#include "models/Problem.h"
#include "models/Vehicle.h"
#include "models/Client.h"
#include "models/Depot.h"
#include "../../data/attributes/Rendezvous.hpp"
namespace CVRPTW{
    class Reader
            : public routing::Reader
    {
    public:
        virtual CVRPTW::Problem *readFile(std::string filepath);
    };
}

#endif //HYBRID_READER_H
