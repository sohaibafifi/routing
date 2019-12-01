//
// Created by Sohaib LAFIFI on 29/11/2019.
//

#pragma once

#include <vrp/models/Solution.hpp>
#include "Tour.hpp"

namespace cvrptw {
    namespace models {
        class Solution : public vrp::models::Solution {
        public :


            explicit Solution(routing::Problem *p_problem)
                    : vrp::models::Solution(p_problem) {
            }

            void update() override {}

        };
    }
}


