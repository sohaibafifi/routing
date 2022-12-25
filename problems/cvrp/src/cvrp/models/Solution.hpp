// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <vrp/models/Solution.hpp>
#include "Tour.hpp"

namespace cvrp {
    namespace models {
        class Solution : public vrp::models::Solution {
        public :

            explicit Solution(routing::Problem *p_problem)
                    : vrp::models::Solution(p_problem) {
            }


        };
    }
}


