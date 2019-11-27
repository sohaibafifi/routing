#pragma once

#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
#include "core/routines/operators/Constructor.hpp"

namespace routing {
    class Move : public Neighborhood {
    public :
        Move(Constructor *p_constructor) :
                constructor(p_constructor) {

        }

        virtual bool look(models::Solution *solution) {
            assert(solution->notserved.size() == 0);

            routing::models::Solution *best = solution->clone();
            bool improved = false;
            double bestCost = solution->getCost();
            // in each tour we look for the best remove and try to insert it into the best position
            for (int t = 0; t < solution->getNbTour(); ++t) {
                if (solution->getTour(t)->getNbClient() == 0) continue;
                routing::RemoveCost *bestRemove = solution->getTour(t)->evaluateRemove(0);
                unsigned bestRemove_position = 0;
                for (int p = 1; p < solution->getTour(t)->getNbClient(); ++p) {
                    routing::RemoveCost *cost = solution->getTour(t)->evaluateRemove(p);
                    if (*cost > *bestRemove) {
                        bestRemove_position = p;
                        bestRemove = cost;
                    }
                }
                models::Client *client = solution->getTour(t)->getClient(bestRemove_position);
                solution->removeClient(t, bestRemove_position);
                if (constructor->bestInsertion(solution, client) && solution->getCost() < bestCost - 1e-9) {
                    bestCost = solution->getCost();
                    best = solution->clone();
                    improved = true;
                }

            }

            solution = best->clone();
            return improved;
        }

    private:
        Constructor *constructor;
    };
}
