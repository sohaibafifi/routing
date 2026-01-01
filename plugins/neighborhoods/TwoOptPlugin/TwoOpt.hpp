// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"
#include "core/interfaces/IIncrementalEvaluator.hpp"
#include <cassert>
#include <utility>

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

    /**
     * @brief 2-Opt neighborhood with incremental evaluation support
     *
     * This neighborhood uses cached route state for O(1) feasibility checking
     * when incremental evaluators are available.
     *
     * Complexity improvement:
     * - Before: O(n²) pairs × O(n) feasibility = O(n³)
     * - After: O(n²) pairs × O(1) feasibility = O(n²)
     *
     * For a 100-client route: ~100x speedup
     */
    class TwoOpt : public Neighborhood {
    public :

        virtual bool look(Solution *solution) {
            routing::Solution *best = solution->clone();
            bool improved = false;
            TwoOptMovement bestMovement(0, 0, 0, std::numeric_limits<routing::Duration>::max(), false);
            double bestCost = solution->getCost();
            auto* problem = solution->getProblem();
            const auto& evaluators = problem->getActiveEvaluators();

            // Collect incremental evaluators for O(1) feasibility checks
            std::vector<IIncrementalEvaluator*> incEvaluators;
            for (auto* eval : evaluators) {
                if (auto* inc = dynamic_cast<IIncrementalEvaluator*>(eval)) {
                    incEvaluators.push_back(inc);
                }
            }

            // Fallback to original O(n) method if no incremental evaluators
            auto isTourFeasibleLegacy = [&](const std::vector<models::Client*>& seq, unsigned tourId) -> bool {
                if (evaluators.empty()) {
                    return true;
                }
                auto* tour = new routing::Tour(problem, tourId);
                bool ok = true;
                for (auto* client : seq) {
                    if (!client) {
                        continue;
                    }
                    auto* cost = tour->evaluateInsertion(client, tour->getNbClient());
                    bool possible = cost->isPossible();
                    delete cost;
                    if (!possible) {
                        ok = false;
                        break;
                    }
                    tour->_pushClient(client);
                }
                delete tour;
                return ok;
            };

            auto buildCandidate = [&](routing::Tour* tour, int first, int second) {
                if (first > second) {
                    std::swap(first, second);
                }
                std::vector<models::Client*> seq;
                seq.reserve(tour->getNbClient());
                for (int k = 0; k <= first; ++k) {
                    seq.push_back(tour->getClient(k));
                }
                for (int k = second; k > first; --k) {
                    seq.push_back(tour->getClient(k));
                }
                for (int k = second + 1; k < static_cast<int>(tour->getNbClient()); ++k) {
                    seq.push_back(tour->getClient(k));
                }
                return seq;
            };

            // Check 2-opt feasibility using incremental evaluators (O(1) per check)
            auto isTwoOptFeasibleIncremental = [&](routing::Tour* tour, const RouteCache& cache, int i, int j) -> bool {
                TwoOptContext ctx{i, j};
                for (auto* incEval : incEvaluators) {
                    MoveDelta md = incEval->evaluateTwoOptIncremental(*tour, cache, ctx);
                    if (!md.feasible) {
                        return false;
                    }
                }
                return true;
            };

            // For each tour, evaluate all 2-opt moves
            for (size_t t = 0; t < solution->getNbTour(); ++t) {
                auto* tour = solution->getTour(t);
                if (tour->getNbClient() < 2) continue;

                // Build cache once per tour for O(1) feasibility checks
                bool useIncremental = !incEvaluators.empty();
                if (useIncremental) {
                    tour->ensureCache();
                }
                const RouteCache& cache = tour->getCache();

                for (int i = 0; i < static_cast<int>(tour->getNbClient()) - 1; ++i) {
                    for (int j = i + 2; j < static_cast<int>(tour->getNbClient()); ++j) {
                        // O(1) distance delta calculation
                        double distance_i_i1 = problem->getDistance(
                                *tour->getClient(i),
                                *tour->getClient(i + 1)
                        );

                        double distance_j_j1;
                        if (j + 1 < static_cast<int>(tour->getNbClient())) {
                            distance_j_j1 = problem->getDistance(
                                    *tour->getClient(j),
                                    *tour->getClient(j + 1)
                            );
                        } else {
                            distance_j_j1 = problem->getDistance(
                                    *tour->getClient(j),
                                    *problem->getDepot()
                            );
                        }

                        double distance_i_j = problem->getDistance(
                                *tour->getClient(i),
                                *tour->getClient(j)
                        );
                        double distance_i1_j1;
                        if (j + 1 < static_cast<int>(tour->getNbClient())) {
                            distance_i1_j1 = problem->getDistance(
                                    *tour->getClient(i + 1),
                                    *tour->getClient(j + 1)
                            );
                        } else {
                            distance_i1_j1 = problem->getDistance(
                                    *tour->getClient(i + 1),
                                    *problem->getDepot()
                            );
                        }

                        double delta = (distance_i_j + distance_i1_j1) - (distance_i_i1 + distance_j_j1);

                        // Only check feasibility for improving moves
                        if (delta >= -1e-9) continue;

                        // O(1) feasibility check using cache (or O(n) fallback)
                        bool feasible;
                        if (useIncremental && cache.isValid()) {
                            feasible = isTwoOptFeasibleIncremental(tour, cache, i, j);
                        } else {
                            auto candidate = buildCandidate(tour, i, j);
                            feasible = isTourFeasibleLegacy(candidate, tour->getID());
                        }

                        if (!feasible) continue;

                        TwoOptMovement cost(i, j, static_cast<int>(t), delta, true);
                        if (bestMovement > cost) {
                            bestMovement = cost;
                        }
                    }
                }
            }

            // Apply best move if found
            if (bestMovement.isPossible() && bestMovement.getDelta() < -1e-9) {
                routing::Tour *tour = dynamic_cast<routing::Tour*>(solution->getTour(bestMovement.t)->clone());
                tour->clear();
                unsigned first = std::min(bestMovement.i, bestMovement.j);
                unsigned second = std::max(bestMovement.i, bestMovement.j);
                for (unsigned k = 0; k <= first; ++k) {
                    tour->_pushClient(solution->getTour(bestMovement.t)->getClient(k));
                }
                for (int k = second; k > static_cast<int>(first); --k) {
                    tour->_pushClient(solution->getTour(bestMovement.t)->getClient(k));
                }
                for (unsigned k = second + 1; k < solution->getTour(bestMovement.t)->getNbClient(); ++k) {
                    tour->_pushClient(solution->getTour(bestMovement.t)->getClient(k));
                }
                solution->overrideTour(tour, bestMovement.t);
                solution->update();
            }

            if (solution->getCost() < bestCost - 1e-9) {
                bestCost = solution->getCost();
                best->copy(solution);
                improved = true;
            }

            solution->copy(best);
            delete best;
            return improved;
        }
    };
}
