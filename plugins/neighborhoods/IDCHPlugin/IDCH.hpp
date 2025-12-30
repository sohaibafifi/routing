// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Destructor.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Constructor.hpp"
#include <cassert>

namespace routing {
    class IDCH : public Neighborhood {
    public :
        IDCH(Constructor *p_constructor, Destructor *p_destructor) :
                constructor(p_constructor), destructor(p_destructor) {

        }

        virtual bool look(Solution *solution) {
            solution->update();
            // assert(solution->notserved.size() == 0);
            int itermax =  10;
            int iter = 0;
            routing::Solution *best = solution->clone();
            bool improved = false;
            double bestCost = solution->getCost();
            auto* problem = solution->getProblem();
            const auto& evaluators = problem->getActiveEvaluators();

            auto isTourFeasible = [&](routing::Tour* tour) -> bool {
                if (evaluators.empty()) {
                    return true;
                }
                auto* check = new routing::Tour(problem, tour->getID());
                bool ok = true;
                for (unsigned long i = 0; i < tour->getNbClient(); ++i) {
                    auto* client = tour->getClient(i);
                    auto* cost = check->evaluateInsertion(client, check->getNbClient());
                    bool possible = cost->isPossible();
                    delete cost;
                    if (!possible) {
                        ok = false;
                        break;
                    }
                    check->_pushClient(client);
                }
                delete check;
                return ok;
            };

            auto isSolutionFeasible = [&]() -> bool {
                if (evaluators.empty()) {
                    return true;
                }
                if (!solution->notserved.empty()) {
                    return false;
                }
                for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                    auto* tour = solution->getTour(t);
                    if (!tour) {
                        continue;
                    }
                    if (!isTourFeasible(tour)) {
                        return false;
                    }
                }
                return true;
            };

            while (iter < itermax) {
                iter++;
                destructor->destruct(solution);
                if (constructor->bestInsertion(solution) && isSolutionFeasible()) {
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
