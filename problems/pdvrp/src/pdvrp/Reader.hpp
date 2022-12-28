//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once


#include <cvrp/Reader.hpp>

namespace pdvrp {
    class Reader : public cvrp::Reader {
    public :
        routing::Problem *readFile(std::string filePath) override;
    };

}

