#pragma once

#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
#include <cassert>

namespace routing {
    class TwoOpt : public Neighborhood {
    public :

        virtual bool look(models::Solution *solution) {
            assert(solution->notserved.size() == 0);

            routing::models::Solution *best = solution->clone();
            bool improved = false;
            double bestCost = solution->getCost();
            // in each tour we look for the best remove and try to insert it into the best position
            for (int t = 0; t < solution->getNbTour(); ++t) {
                if (solution->getTour(t)->getNbClient() == 0) continue;
                for (int i = 0; i < solution->getTour(t)->getNbClient() - 1; ++i) {
                    for (int j = 0; j < solution->getTour(t)->getNbClient() - 1; ++j) {
                        if (j == i || j == i - 1 || j == i + 1) continue;
                        // check if distance_i_i1 + distance_j_j1 > distance_i_j + distance_i1_j1
                        double distance_i_i1 = solution->getProblem()->getDistance(
                                *solution->getTour(t)->getClient(i),
                                *solution->getTour(t)->getClient(i + 1)
                        );
                        double distance_j_j1 = solution->getProblem()->getDistance(
                                *solution->getTour(t)->getClient(j),
                                *solution->getTour(t)->getClient(j + 1)
                        );
                        double distance_i_j = solution->getProblem()->getDistance(
                                *solution->getTour(t)->getClient(i),
                                *solution->getTour(t)->getClient(j)
                        );
                        double distance_i1_j1 = solution->getProblem()->getDistance(
                                *solution->getTour(t)->getClient(i + 1),
                                *solution->getTour(t)->getClient(j + 1)
                        );
                        if(distance_i_i1 + distance_j_j1 > distance_i_j + distance_i1_j1){
                            routing::models::Tour * tour = solution->getTour(t)->clone();
                            tour->clear();
                            unsigned   first = std::min(i, j), second = std::max(i, j);
                            for (int k = 0; k <= first; ++k) {
                                tour->pushClient(solution->getTour(t)->getClient(k));
                            }
                            for (int k = second; k > first; --k) {
                                tour->pushClient(solution->getTour(t)->getClient(k));
                            }
                            for (int k = second; k < solution->getTour(t)->getNbClient(); ++k) {
                                tour->pushClient(solution->getTour(t)->getClient(k));
                            }
                            solution->overrideTour(tour, t);
                        }


                    }
                }
                if (solution->getCost() < bestCost - 1e-9) {
                        bestCost = solution->getCost();
                        best->copy(solution);
                        improved = true;
                    }

            }

            solution->copy(best);
            return improved;
        }


    };
}
