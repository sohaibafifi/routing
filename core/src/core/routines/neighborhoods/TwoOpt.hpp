// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
#include <cassert>

namespace routing {
    class TwoOptMovement {
    private:
        routing::Duration delta;
        bool possible;
    public:
        int i, j, t;

        bool operator>(const TwoOptMovement &rhs) const {
            return delta > rhs.delta;
        }


        routing::Duration getDelta() const {
            return delta;
        }

        void setDelta(routing::Duration delta) {
            TwoOptMovement::delta = delta;
        }

        bool isPossible() const {
            return possible;
        }

        void setPossible(bool possible) {
            TwoOptMovement::possible = possible;
        }

        TwoOptMovement(int first_point, int second_point, int tour, routing::Duration delta, bool possible) :
                i(first_point), j(second_point), delta(delta), t(tour), possible(possible) {}

        TwoOptMovement() : delta(0), possible(true) {}

        TwoOptMovement(const TwoOptMovement &cost) : delta(cost.getDelta()), possible(cost.isPossible()) {}
    };

    class TwoOpt : public Neighborhood {
    public :

        virtual bool look(models::Solution *solution) {
            // assert(solution->notserved.size() == 0);

            routing::models::Solution *best = solution->clone();
            bool improved = false;
            TwoOptMovement bestMovement(0, 0, 0, std::numeric_limits<routing::Duration>::max(), false);
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
                        double delta = distance_i_i1 + distance_j_j1 - (distance_i_j + distance_i1_j1);
                        TwoOptMovement cost(i, j, t, delta, true);
                        if (delta < 0 && bestMovement > cost) {
                            bestMovement = cost;

                        }
                    }
                }
            }
            if (bestMovement.isPossible()) {
                routing::models::Tour *tour = solution->getTour(bestMovement.t)->clone();
                tour->clear();
                unsigned first = std::min(bestMovement.i, bestMovement.j), second = std::max(bestMovement.i,
                                                                                             bestMovement.j);
                for (int k = 0; k <= first; ++k) {
                    tour->_pushClient(solution->getTour(bestMovement.t)->getClient(k));
                }
                for (int k = second; k > first; --k) {
                    tour->_pushClient(solution->getTour(bestMovement.t)->getClient(k));
                }
                for (int k = second; k < solution->getTour(bestMovement.t)->getNbClient(); ++k) {
                    tour->_pushClient(solution->getTour(bestMovement.t)->getClient(k));
                }
                solution->overrideTour(tour, bestMovement.t);
            }
            if (solution->getCost() < bestCost - 1e-9) {
                bestCost = solution->getCost();
                best->copy(solution);
                improved = true;
            }


            solution->copy(best);
            return improved;
        }


    };
}
