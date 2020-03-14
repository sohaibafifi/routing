// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Problem.hpp"

namespace routing {

    class Reader {
    public :
        virtual Problem *readFile(std::string filepath) = 0;

    };

}
