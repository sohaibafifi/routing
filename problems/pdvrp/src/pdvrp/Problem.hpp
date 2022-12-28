// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrp/Problem.hpp>
namespace pdvrp {
    class Problem : public cvrp::Problem {
    public:
        Problem() : cvrp::Problem() {  };
        explicit Problem(const cvrp::Problem & p_problem) : cvrp::Problem(p_problem){  };

    protected:

        void addCapacityConstraints() override;

    public:
        routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;

    protected :

    };
}


