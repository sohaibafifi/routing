// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <vrp/Problem.hpp>
#include "models/Solution.hpp"
#include "models/Tour.hpp"

namespace cvrp {
    class Problem : public vrp::Problem {
    public:
        Problem() = default;
    protected :
        routing::Initializer *initializer() override;

#ifdef CPLEX
        std::vector<IloNumVar> consumption;
        virtual void addConstraints() override;
        virtual void addVariables() override ;

        virtual void addCapacityConstraints();
#endif

    };

    class Initializer : public routing::Initializer {
    public:
        Initializer(Problem *p_problem) :
                routing::Initializer(p_problem) {

        }

        models::Solution *initialSolution() override {
            return new models::Solution(this->getProblem());
        }

        cvrp::models::Tour *initialTour(int vehicleID) override {
            return new models::Tour(this->getProblem(), vehicleID);
        }
    };
}


