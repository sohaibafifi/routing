// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrp/models/Solution.hpp>
#include "Tour.hpp"

namespace cvrptw {
    namespace models {
        class Solution : public cvrp::models::Solution {
        public :

            explicit Solution(routing::Problem *p_problem)
                    : cvrp::models::Solution(p_problem) {
            }

            void update() override {}

        };
    }
}


