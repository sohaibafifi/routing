//
// Created by Sohaib LAFIFI on 29/11/2019.
//

#pragma once

#include <vrp/Problem.hpp>
#include "models/Solution.hpp"

namespace cvrptw {

    class Problem : public vrp::Problem{
        routing::models::Solution * initialSolution() override{
            return new models::Solution(this);
        }

    };
}


