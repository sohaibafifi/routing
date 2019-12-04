//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once

#include "Problem.hpp"
#include <core/data/Reader.hpp>

namespace vrp {
    class Reader : public routing::Reader {
    public :
        vrp::Problem *readFile(std::string filepath) override;
    };

}

