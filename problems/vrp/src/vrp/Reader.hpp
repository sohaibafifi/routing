// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Problem.hpp"
#include <core/data/Reader.hpp>

namespace vrp {
    class Reader : public routing::Reader {
    public :
        vrp::Problem *readFile(std::string filepath) override;
    };

}

