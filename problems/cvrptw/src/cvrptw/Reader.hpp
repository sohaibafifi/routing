//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once


#include <core/data/Reader.hpp>

namespace cvrptw {
    class Reader : public routing::Reader {
    public :
        routing::Problem *readFile(std::string filepath) override;
    };
}
