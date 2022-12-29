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
        std::vector<IloNumVar> consumption;
    protected :
        virtual routing::Initializer *initializer() override;


        virtual void addConstraints() override;
        virtual void addVariables() override ;

        virtual void addCapacityConstraints();

    };

    class Initializer : public routing::Initializer {
    public:
        explicit Initializer(Problem *p_problem) :
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


