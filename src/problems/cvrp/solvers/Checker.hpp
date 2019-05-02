//
// Created by ali on 4/15/19.
//

#pragma once

#include "../../../solvers/Checker.hpp"
#include "../models/Solution.hpp"

namespace CVRP{
    class Checker : public routing::Checker
    {
        public:
            virtual bool check() override;
            Checker(routing::models::Solution* solution, std::ostream& os): routing::Checker(solution,os){}

    };

}
