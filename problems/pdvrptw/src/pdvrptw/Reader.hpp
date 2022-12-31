//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once


#include <cvrptw/Reader.hpp>

namespace pdvrptw {
    class Reader : public cvrptw::Reader {
    public :
        routing::Problem *readFile(const std::string & filepath) override;
    };

}

