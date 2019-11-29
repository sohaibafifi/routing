#pragma once

#include <random>
#include "../../data/models/Solution.hpp"
#include "core/routines/neighborhoods/Neighborhood.hpp"
#include "Destructor.hpp"
#include "Constructor.hpp"

namespace routing {
    class Generator {
    public :
        Generator(Problem *p_problem, Constructor *p_constructor, Destructor *p_destructor) :
                problem(p_problem),
                constructor(p_constructor), destructor(p_destructor) {

        }

        virtual models::Solution *initialSolution() {
            return this->problem->initializer()->initialSolution();
        };

        virtual models::Solution *generate() {
            models::Solution *solution = this->initialSolution();
            solution->notserved.clear();
            for (unsigned long c = 0; c < problem->clients.size(); ++c) {
                solution->notserved.push_back(problem->clients[c]);
            }
            std::random_device rd;
            std::default_random_engine g(rd());
            std::shuffle(solution->notserved.begin(), solution->notserved.end(), g);
            unsigned iter = 1;
            while (iter < problem->clients.size()) {
                if (constructor->bestInsertion(solution)) {
                    return solution;
                } else {
                    iter++;
                    destructor->destruct(solution);
                }
            }
            return nullptr;
        }

    protected:
        Constructor *constructor;
        Destructor *destructor;
        routing::Problem *problem;
    };

}
