// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrptw/Problem.hpp>

namespace pdvrptw {
    class Problem : public cvrptw::Problem {
    public:
        Problem() : cvrptw::Problem() {  };
        explicit Problem(const cvrptw::Problem & p_problem) : cvrptw::Problem(p_problem){  };

    protected:
#ifdef CPLEX_FOUND

        void addCapacityConstraints() override;

    public:
        routing::callback::HeuristicCallback *setHeuristicCallback() override;
#endif
    protected :

    };
}


