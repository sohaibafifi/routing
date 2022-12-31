// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
#include "core/routines/operators/Destructor.hpp"
#include "core/routines/operators/Constructor.hpp"
#include <cassert>

namespace routing {
    class IDCH : public Neighborhood {
    public :
        IDCH(Constructor *p_constructor, Destructor *p_destructor) :
                constructor(p_constructor), destructor(p_destructor) {

        }

        virtual bool look(models::Solution *solution) {
            solution->update();
            // assert(solution->notserved.size() == 0);
            int itermax =  10;
            int iter = 0;
            routing::models::Solution *best = solution->clone();
            bool improved = false;
            double bestCost = solution->getCost();

            while (iter < itermax) {
                iter++;
                destructor->destruct(solution);
                if (constructor->bestInsertion(solution)) {
                    if (solution->getCost() < bestCost - 1e-9) {
                        solution->update(); // FIXME : remove
                        bestCost = solution->getCost();
                        iter = 1;
                        best->copy(solution);
                        improved = true;
                    }
                } else
                    solution->copy(best);

            }
            solution->copy(best);
            return improved;

        }


    protected:
        Constructor *constructor;
        Destructor *destructor;
    };

}
