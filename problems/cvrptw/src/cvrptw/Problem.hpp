// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrp/Problem.hpp>
#include "models/Solution.hpp"
#include "models/Tour.hpp"

namespace cvrptw {

    class Problem : public cvrp::Problem {

    public :

        std::vector<IloNumVar> start;
    protected :
        virtual routing::Initializer *initializer() override;

        virtual void addVariables() override;


        virtual void addSequenceConstraints() override;

        virtual void addAffectationConstraints() override;

        virtual void addCapacityConstraints() override;

    };

    class Initializer : public routing::Initializer {
    public:
        explicit Initializer(Problem *p_problem) :
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


