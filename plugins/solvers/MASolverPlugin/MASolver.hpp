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
#include "plugins/solvers/OperatorsPlugin/operators/GreedyConstructor.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/RandomDestructor.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"
#include "plugins/neighborhoods/IDCHPlugin/IDCH.hpp"
#include "plugins/neighborhoods/TwoOptPlugin/TwoOpt.hpp"
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
            std::vector<Neighborhood *> active = this->neighbors;
            static GreedyConstructor constructor;
            static RandomDestructor destructor(0.2);
            static IDCH idch(&constructor, &destructor);
            static TwoOpt twoopt;
            if (active.empty()) {
                active.push_back(&idch);
                active.push_back(&twoopt);
            }
            std::vector<bool> run(active.size(), false);
            std::random_device rd;
            while (std::find(run.begin(), run.end(), false) != run.end()) {
                unsigned i = 0;
                do { i = rd() % run.size(); } while (run[i]);
                if (active[i]->look(solution)) {
                    run = std::vector<bool>(active.size(), false);
                } else {
                    run[i] = true;
                }
            }
            sequence->sequence = solution->getSequence();
        }


    };
}
