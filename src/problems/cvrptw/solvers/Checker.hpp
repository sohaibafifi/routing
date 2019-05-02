//
// Created by ali on 4/15/19.
//

#pragma once

#include "../../cvrp/solvers/Checker.hpp"
#include "../models/Solution.hpp"

namespace CVRPTW{
    class Checker : public CVRP::Checker {
        public:
            virtual bool check() override;

            Checker(routing::models::Solution* solution, std::ostream& os): CVRP::Checker(solution,os){}


    };

}
