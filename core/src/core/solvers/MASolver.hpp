//
// Created by Sohaib LAFIFI on 22/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "GASolver.hpp"
#include "Solver.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/neighborhoods/Neighborhood.hpp"
#include <cassert>
#include <algorithm>
#include <set>
#include <utilities/Utilities.hpp>

namespace routing {


    template<class Reader>
    class MASolver : public GASolver<Reader> {

    public:
        MASolver(const std::string &p_inputFile,
                 Generator *p_generator,
                 const std::vector<Neighborhood *> &p_neighbors,
                 std::ostream &os = std::cout) : GASolver<Reader>(p_inputFile, p_generator, p_neighbors, os) {
        }

        MASolver(const std::string &p_inputFile,
                 std::ostream &os = std::cout) : GASolver<Reader>(p_inputFile, os) {
        }

        void mutate(Sequence *sequence) override {
            models::Solution * solution = sequence->decode();
            assert(solution->notserved.empty());
            std::vector<bool> run(this->neighbors.size(), false);
            std::random_device rd;
            while (std::find(run.begin(), run.end(), false) != run.end()) {
                unsigned i = 0;
                do { i = rd() % run.size(); } while (run[i]);
                this->os << typeid(this->neighbors[i]).name() << std::endl;
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
