#pragma once

#include <random>
#include "../../data/models/Solution.hpp"
#include "core/routines/neighborhoods/Neighborhood.hpp"
#include "Destructor.hpp"
#include "Constructor.hpp"

namespace routing {
    class Generator {
    public :
        Generator(Constructor *p_constructor, Destructor *p_destructor) :
                constructor(p_constructor), destructor(p_destructor) {

        }

        virtual bool generate(models::Solution *solution) {
            solution->notserved.clear();
            for (unsigned long c = 0; c < solution->getProblem()->clients.size(); ++c) {
                solution->notserved.push_back(solution->getProblem()->clients[c]);
            }
            std::random_device rd;
            std::default_random_engine g(rd());
            std::shuffle(solution->notserved.begin(), solution->notserved.end(), g);
            unsigned iter = 1;
            while (iter < solution->getProblem()->clients.size()) {
                if (constructor->bestInsertion(solution)) {
                    return true;
                } else {
                    iter++;
                    destructor->destruct(solution);
                }
            }
            return false;
        }

    private:
        Constructor *constructor;
        Destructor *destructor;
    };

}
