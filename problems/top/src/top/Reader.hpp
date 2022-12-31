//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once


#include <cvrp/Reader.hpp>

namespace top {
    class Reader : public cvrp::Reader {
    public :
        routing::Problem *readFile(const std::string & filepath) override;
    };

}

