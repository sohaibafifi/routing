//
// Created by Sohaib LAFIFI on 22/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "plugins/solvers/GASolverPlugin/GASolver.hpp"
#include "plugins/solvers/SolverCorePlugin/Solver.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Generator.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"
#include <cassert>
#include <algorithm>
#include <set>


namespace routing {

    class MASolver : public GASolver {

    public:
        MASolver(routing::Problem *p_problem,
                 Generator *p_generator,
                 const std::vector<Neighborhood *> &p_neighbors,
                 std::ostream &os = std::cout)
            : GASolver(p_problem, p_generator, p_neighbors, os) {
        }

        MASolver(routing::Problem *p_problem,
                 std::ostream &os = std::cout)
            : GASolver(p_problem, os) {
        }

        void mutate(Sequence *sequence) override {
            Solution * solution = sequence->decode();
            assert(solution->notserved.empty());
            std::vector<bool> run(this->neighbors.size(), false);
            std::random_device rd;
            while (std::find(run.begin(), run.end(), false) != run.end()) {
                unsigned i = 0;
                do { i = rd() % run.size(); } while (run[i]);
                if (this->neighbors[i]->look(solution)) {
                    run = std::vector<bool>(this->neighbors.size(), false);
                } else {
                    run[i] = true;
                }
            }
            sequence->sequence = solution->getSequence();
        }


    };
}
