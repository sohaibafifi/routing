// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Problem.hpp"
#include <sstream>

namespace routing {

    class Reader {
    public :
        virtual Problem *readFile(const std::string &filepath) = 0;

        inline static std::vector<std::string> splitString(std::string line, char sep) {
            std::istringstream ss(line);
            std::string token;
            std::vector<std::string> split;
            while (std::getline(ss, token, sep)) {
                split.push_back(token);
            }
            return split;
        }
    };

}
