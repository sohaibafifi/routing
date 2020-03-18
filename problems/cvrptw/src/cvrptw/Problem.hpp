// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <vrp/Problem.hpp>
#include "models/Solution.hpp"
#include "models/Tour.hpp"

namespace cvrptw {
    class Problem : public vrp::Problem {
    protected :
        routing::Initializer *initializer() override;

#ifdef CPLEX
        void addVariables() override;

        void addSequenceConstraints() override;

        std::vector<IloNumVar> start;
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

        cvrptw::models::Tour *initialTour(int vehicleID) override {
            return new models::Tour(this->getProblem(), vehicleID);
        }
    };
}


