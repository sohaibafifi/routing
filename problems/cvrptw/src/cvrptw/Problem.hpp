//
// Created by Sohaib LAFIFI on 29/11/2019.
//

#pragma once

#include <vrp/Problem.hpp>
#include "models/Solution.hpp"
#include "models/Tour.hpp"

namespace cvrptw {
    class Problem : public vrp::Problem{
          routing::Initializer * initializer() override ;
    };
    class  Initializer : public routing::Initializer{
      public:
        Initializer(Problem *p_problem):
            routing::Initializer(p_problem){

        }
         models::Solution * initialSolution() override {
            return new models::Solution(this->getProblem());
        }
         cvrptw::models::Tour * initialTour(int vehicleID) override {
                return new models::Tour(this->getProblem(), vehicleID);
         }
    };
}


