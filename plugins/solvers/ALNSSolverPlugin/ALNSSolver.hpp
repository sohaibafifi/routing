//
// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "plugins/solvers/SolverCorePlugin/Solver.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Generator.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/GreedyConstructor.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/RandomDestructor.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"
#include "plugins/neighborhoods/TwoOptPlugin/TwoOpt.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Destructor.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Constructor.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>
#include <numeric>
#include <limits>

namespace routing {

    class DestroyOperator {
    public:
        std::string name;
        double weight;
        double score;
        int used;
        int successes;

        DestroyOperator(const std::string& p_name, double p_weight = 1.0)
            : name(p_name), weight(p_weight), score(0.0), used(0), successes(0) {}

        virtual void destruct(Solution *solution, Problem *problem) = 0;
        virtual ~DestroyOperator() = default;

        void updateScore(double improvement) {
            used++;
            if (improvement > 1e-9) {
                score += improvement;
                successes++;
            }
        }

        void updateWeight(double reactionFactor, double decayFactor) {
            double newWeight = weight * (1.0 - decayFactor) + decayFactor * score;
            weight = std::max(0.1, newWeight);
            score *= (1.0 - reactionFactor);
        }
    };

    class RepairOperator {
    public:
        std::string name;
        double weight;
        double score;
        int used;
        int successes;

        RepairOperator(const std::string& p_name, double p_weight = 1.0)
            : name(p_name), weight(p_weight), score(0.0), used(0), successes(0) {}

        virtual bool repair(Solution *solution, Problem *problem) = 0;
        virtual ~RepairOperator() = default;

        void updateScore(double improvement) {
            used++;
            if (improvement > 1e-9) {
                score += improvement;
                successes++;
            }
        }

        void updateWeight(double reactionFactor, double decayFactor) {
            double newWeight = weight * (1.0 - decayFactor) + decayFactor * score;
            weight = std::max(0.1, newWeight);
            score *= (1.0 - reactionFactor);
        }
    };

    class RandomRemoval : public DestroyOperator {
    private:
        std::mt19937 &gen_;
        double removalFraction_;

    public:
        RandomRemoval(std::mt19937 &gen, double fraction = 0.2)
            : DestroyOperator("RandomRemoval"), gen_(gen), removalFraction_(fraction) {}

        void destruct(Solution *solution, Problem *problem) override {
            std::vector<std::pair<unsigned long, unsigned long>> positions;
            for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                auto* tour = solution->getTour(t);
                if (!tour) continue;
                for (unsigned long p = 0; p < tour->getNbClient(); ++p) {
                    positions.emplace_back(t, p);
                }
            }

            if (positions.empty()) return;

            size_t removeCount = static_cast<size_t>(std::max(1.0, positions.size() * removalFraction_));
            removeCount = std::min(removeCount, positions.size());

            std::shuffle(positions.begin(), positions.end(), gen_);
            positions.resize(removeCount);

            std::sort(positions.begin(), positions.end(),
                      [](const auto& a, const auto& b) {
                          if (a.first != b.first) return a.first < b.first;
                          return a.second > b.second;
                      });

            for (const auto& pos : positions) {
                solution->removeClient(pos.first, pos.second);
            }
        }
    };

    class WorstRemoval : public DestroyOperator {
    private:
        std::mt19937 &gen_;
        double removalFraction_;
        double randomness_;

    public:
        WorstRemoval(std::mt19937 &gen, double fraction = 0.2, double randomness = 0.1)
            : DestroyOperator("WorstRemoval"), gen_(gen), removalFraction_(fraction), randomness_(randomness) {}

        void destruct(Solution *solution, Problem *problem) override {
            std::vector<std::pair<double, std::pair<unsigned long, unsigned long>>> costs;

            for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                auto* tour = solution->getTour(t);
                if (!tour) continue;

                for (unsigned long p = 0; p < tour->getNbClient(); ++p) {
                    double costBefore = tour->getCost();
                    auto* client = tour->getClient(p);
                    solution->removeClient(t, p);
                    double costAfter = solution->getCost();

                    double removalCost = costBefore - costAfter;
                    costs.emplace_back(-removalCost, std::make_pair(t, p));

                    auto* entityClient = dynamic_cast<Entity*>(client);
                    if (entityClient) {
                        tour->addClient(client, static_cast<unsigned>(p), new InsertionCost(0, true));
                    }
                    solution->update();
                }
            }

            if (costs.empty()) return;

            size_t removeCount = static_cast<size_t>(std::max(1.0, costs.size() * removalFraction_));
            removeCount = std::min(removeCount, costs.size());

            std::sort(costs.begin(), costs.end());

            std::vector<std::pair<unsigned long, unsigned long>> toRemove;
            std::uniform_real_distribution<> dis(0.0, 1.0);

            for (size_t i = 0; i < removeCount && i < costs.size(); ++i) {
                if (dis(gen_) < randomness_) {
                    size_t randomIdx = std::uniform_int_distribution<size_t>(i, costs.size() - 1)(gen_);
                    toRemove.push_back(costs[randomIdx].second);
                    costs.erase(costs.begin() + randomIdx);
                } else {
                    toRemove.push_back(costs[i].second);
                }
            }

            std::sort(toRemove.begin(), toRemove.end(),
                      [](const auto& a, const auto& b) {
                          if (a.first != b.first) return a.first < b.first;
                          return a.second > b.second;
                      });

            for (const auto& pos : toRemove) {
                solution->removeClient(pos.first, pos.second);
            }
        }
    };

    class ShawRemoval : public DestroyOperator {
    private:
        std::mt19937 &gen_;
        double removalFraction_;
        double alpha_;
        double beta_;
        double gamma_;

    public:
        ShawRemoval(std::mt19937 &gen, double fraction = 0.2, double alpha = 0.5, double beta = 0.5, double gamma = 0.5)
            : DestroyOperator("ShawRemoval"), gen_(gen), removalFraction_(fraction),
              alpha_(alpha), beta_(beta), gamma_(gamma) {}

        void destruct(Solution *solution, Problem *problem) override {
            std::vector<std::pair<unsigned long, unsigned long>> allPositions;
            for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                auto* tour = solution->getTour(t);
                if (!tour) continue;
                for (unsigned long p = 0; p < tour->getNbClient(); ++p) {
                    allPositions.emplace_back(t, p);
                }
            }

            if (allPositions.empty()) return;

            size_t removeCount = static_cast<size_t>(std::max(1.0, allPositions.size() * removalFraction_));
            removeCount = std::min(removeCount, allPositions.size());

            std::uniform_int_distribution<size_t> seedDist(0, allPositions.size() - 1);
            size_t seedIdx = seedDist(gen_);
            auto seedPos = allPositions[seedIdx];
            auto* seedClient = solution->getTour(seedPos.first)->getClient(seedPos.second);

            std::vector<std::pair<double, std::pair<unsigned long, unsigned long>>> distances;

            for (const auto& pos : allPositions) {
                if (pos == seedPos) continue;
                auto* client = solution->getTour(pos.first)->getClient(pos.second);

                double distance = 0.0;
                auto* entitySeed = dynamic_cast<Entity*>(seedClient);
                auto* entityClient = dynamic_cast<Entity*>(client);

                if (entitySeed && entityClient && entitySeed->hasAttribute<attributes::GeoNode>() && entityClient->hasAttribute<attributes::GeoNode>()) {
                    distance += alpha_ * problem->getDistanceEntity(*entitySeed, *entityClient);
                }

                if (entitySeed && entityClient && entitySeed->hasAttribute<attributes::Consumer>() && entityClient->hasAttribute<attributes::Consumer>()) {
                    double demand1 = entitySeed->getAttribute<attributes::Consumer>().getDemand();
                    double demand2 = entityClient->getAttribute<attributes::Consumer>().getDemand();
                    distance += beta_ * std::abs(demand1 - demand2);
                }

                if (entitySeed && entityClient && entitySeed->hasAttribute<attributes::Rendezvous>() && entityClient->hasAttribute<attributes::Rendezvous>()) {
                    auto tw1 = entitySeed->getAttribute<attributes::Rendezvous>().getTw();
                    auto tw2 = entityClient->getAttribute<attributes::Rendezvous>().getTw();
                    distance += gamma_ * std::abs(tw1.first - tw2.first);
                }

                distances.emplace_back(distance, pos);
            }

            std::sort(distances.begin(), distances.end());

            std::vector<std::pair<unsigned long, unsigned long>> toRemove;
            toRemove.push_back(seedPos);

            std::uniform_real_distribution<> dis(0.0, 1.0);
            size_t remaining = removeCount - 1;

            for (size_t i = 0; i < distances.size() && toRemove.size() < removeCount; ++i) {
                if (dis(gen_) < 0.5) {
                    toRemove.push_back(distances[i].second);
                }
            }

            std::sort(toRemove.begin(), toRemove.end(),
                      [](const auto& a, const auto& b) {
                          if (a.first != b.first) return a.first < b.first;
                          return a.second > b.second;
                      });

            for (const auto& pos : toRemove) {
                solution->removeClient(pos.first, pos.second);
            }
        }
    };

    class RouteRemoval : public DestroyOperator {
    private:
        std::mt19937 &gen_;
        int maxRoutes_;

    public:
        RouteRemoval(std::mt19937 &gen, int maxRoutes = 1)
            : DestroyOperator("RouteRemoval"), gen_(gen), maxRoutes_(maxRoutes) {}

        void destruct(Solution *solution, Problem *problem) override {
            std::vector<unsigned long> routeIndices;
            for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                if (solution->getTour(t) && solution->getTour(t)->getNbClient() > 0) {
                    routeIndices.push_back(t);
                }
            }

            if (routeIndices.empty()) return;

            int routesToRemove = std::min(maxRoutes_, static_cast<int>(routeIndices.size()));

            std::shuffle(routeIndices.begin(), routeIndices.end(), gen_);
            routeIndices.resize(routesToRemove);

            for (unsigned long routeIdx : routeIndices) {
                auto* tour = solution->getTour(routeIdx);
                while (tour->getNbClient() > 0) {
                    tour->removeClient(tour->getNbClient() - 1);
                }
            }

            solution->update();
        }
    };

    class GreedyRepair : public RepairOperator {
    private:
        std::mt19937 &gen_;

    public:
        GreedyRepair(std::mt19937 &gen) : RepairOperator("GreedyRepair"), gen_(gen) {}

        bool repair(Solution *solution, Problem *problem) override {
            GreedyConstructor constructor;
            return constructor.bestInsertion(solution, solution->notserved);
        }
    };

    class RegretRepair : public RepairOperator {
    private:
        std::mt19937 &gen_;
        int k_;

        double evaluateInsertion(Solution* solution, models::Client* client, unsigned long tourIdx, unsigned long pos) {
            Solution* testSolution = solution->clone();
            testSolution->addClient(tourIdx, client, pos, new InsertionCost(0, true));
            double cost = testSolution->getCost();
            delete testSolution;
            return cost;
        }

    public:
        RegretRepair(std::mt19937 &gen, int k = 2) : RepairOperator("RegretRepair"), gen_(gen), k_(k) {}

        bool repair(Solution *solution, Problem *problem) override {
            if (solution->notserved.empty()) return true;

            std::vector<std::pair<double, models::Client*>> regretScores;

            for (auto* client : solution->notserved) {
                std::vector<double> insertionCosts;

                for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                    auto* tour = solution->getTour(t);
                    if (!tour) continue;

                    for (unsigned long p = 0; p <= tour->getNbClient(); ++p) {
                        double cost = evaluateInsertion(solution, client, t, p);
                        if (cost < std::numeric_limits<double>::infinity()) {
                            insertionCosts.push_back(cost);
                        }
                    }
                }

                if (insertionCosts.size() < 2) {
                    regretScores.emplace_back(0.0, client);
                    continue;
                }

                std::sort(insertionCosts.begin(), insertionCosts.end());
                double regret = 0.0;
                for (int i = 1; i < std::min(k_, static_cast<int>(insertionCosts.size())); ++i) {
                    regret += insertionCosts[i] - insertionCosts[0];
                }
                regretScores.emplace_back(-regret, client);
            }

            if (regretScores.empty()) return false;

            std::sort(regretScores.begin(), regretScores.end());
            auto* clientToInsert = regretScores[0].second;

            double bestCost = std::numeric_limits<double>::infinity();
            unsigned long bestTour = 0;
            unsigned long bestPos = 0;
            bool found = false;

            for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                auto* tour = solution->getTour(t);
                if (!tour) continue;

                for (unsigned long p = 0; p <= tour->getNbClient(); ++p) {
                    double cost = evaluateInsertion(solution, clientToInsert, t, p);
                    if (cost < bestCost) {
                        bestCost = cost;
                        bestTour = t;
                        bestPos = p;
                        found = true;
                    }
                }
            }

            if (found) {
                solution->addClient(bestTour, clientToInsert, bestPos, new InsertionCost(0, true));
                return true;
            }

            return false;
        }
    };

    class RandomRepair : public RepairOperator {
    private:
        std::mt19937 &gen_;

        bool canInsert(Solution* solution, models::Client* client, unsigned long tourIdx, unsigned long pos) {
            Solution* testSolution = solution->clone();
            testSolution->addClient(tourIdx, client, pos, new InsertionCost(0, true));
            delete testSolution;
            return true;
        }

    public:
        RandomRepair(std::mt19937 &gen) : RepairOperator("RandomRepair"), gen_(gen) {}

        bool repair(Solution *solution, Problem *problem) override {
            if (solution->notserved.empty()) return true;

            std::shuffle(solution->notserved.begin(), solution->notserved.end(), gen_);

            for (auto* client : solution->notserved) {
                std::vector<std::pair<unsigned long, unsigned long>> validPositions;

                for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                    auto* tour = solution->getTour(t);
                    if (!tour) continue;

                    for (unsigned long p = 0; p <= tour->getNbClient(); ++p) {
                        if (canInsert(solution, client, t, p)) {
                            validPositions.emplace_back(t, p);
                        }
                    }
                }

                if (!validPositions.empty()) {
                    std::uniform_int_distribution<size_t> posDist(0, validPositions.size() - 1);
                    auto selected = validPositions[posDist(gen_)];
                    solution->addClient(selected.first, client, selected.second, new InsertionCost(0, true));
                }
            }

            return solution->notserved.empty();
        }
    };

    class ALNSSolver : public Solver {
    protected:
        std::vector<DestroyOperator*> destroyOps;
        std::vector<RepairOperator*> repairOps;
        std::vector<routing::Neighborhood*> neighbors;
        routing::Generator *generator = nullptr;
        std::mt19937 gen;

        double reactionFactor_;
        double decayFactor_;
        double temperature_;
        double coolingRate_;
        int segmentSize_;
        double minTemperature_;

    public:
        explicit ALNSSolver(routing::Problem *p_problem,
                   std::ostream &os = std::cout)
            : Solver(p_problem, os),
              gen(std::chrono::steady_clock::now().time_since_epoch().count()),
              reactionFactor_(0.1), decayFactor_(0.8), temperature_(100.0),
              coolingRate_(0.9995), segmentSize_(100), minTemperature_(0.01) {
            this->setDefaultConfiguration();
            initializeOperators();
        }

        ALNSSolver(routing::Problem *p_problem,
                   Generator *p_generator,
                   const std::vector<Neighborhood*> &p_neighbors,
                   std::ostream &os = std::cout)
            : Solver(p_problem, os),
              generator(p_generator),
              neighbors(p_neighbors),
              gen(std::chrono::steady_clock::now().time_since_epoch().count()),
              reactionFactor_(0.1), decayFactor_(0.8), temperature_(100.0),
              coolingRate_(0.9995), segmentSize_(100), minTemperature_(0.01) {
            this->setDefaultConfiguration();
            initializeOperators();
        }

        virtual ~ALNSSolver() {
            for (auto* op : destroyOps) {
                delete op;
            }
            for (auto* op : repairOps) {
                delete op;
            }
        }

        void setGenerator(Generator *p_generator) { this->generator = p_generator; }
        void setNeighbors(std::vector<routing::Neighborhood*> p_neighbors) { this->neighbors = p_neighbors; }

        void setReactionFactor(double factor) { reactionFactor_ = factor; }
        void setDecayFactor(double factor) { decayFactor_ = factor; }
        void setTemperature(double temp) { temperature_ = temp; }
        void setCoolingRate(double rate) { coolingRate_ = rate; }
        void setSegmentSize(int size) { segmentSize_ = size; }
        void setMinTemperature(double temp) { minTemperature_ = temp; }

        void setDefaultConfiguration() override {
            this->configuration = new Configuration();
            this->configuration->setIntParam(this->configuration->iterMax,
                    this->problem->clients.size() * this->problem->clients.size());
        };

        bool solve(double timeout = 3600) override {
            GreedyConstructor fallbackConstructor;
            RandomDestructor fallbackDestructor(0.2);
            Generator fallbackGenerator(this->problem, &fallbackConstructor, &fallbackDestructor);
            Generator* activeGenerator = generator ? generator : &fallbackGenerator;
            auto* previousGenerator = generator;
            generator = activeGenerator;

            std::vector<Neighborhood*> activeNeighbors = neighbors;
            static TwoOpt fallbackTwoOpt;
            if (activeNeighbors.empty()) {
                activeNeighbors.push_back(&fallbackTwoOpt);
            }

            this->solution = activeGenerator->generate();

            Solution* currentSolution = this->solution->clone();
            Solution* bestSolution = this->solution->clone();
            double currentCost = currentSolution->getCost();
            double bestCost = bestSolution->getCost();

            int itermax = this->configuration->getIntParam(this->configuration->iterMax);
            int iter = 1;
            int segmentIter = 0;
            double currentTemperature = temperature_;

            const auto start = std::chrono::steady_clock::now();

            while (iter++ < itermax) {
                if (timeout > 0) {
                    const auto elapsed = std::chrono::duration<double>(
                        std::chrono::steady_clock::now() - start).count();
                    if (elapsed >= timeout) {
                        break;
                    }
                }

                Solution* newSolution = currentSolution->clone();
                destroyAndRepair(newSolution);

                double newCost = newSolution->getCost();
                double delta = newCost - currentCost;

                bool accepted = false;
                if (delta < -1e-9) {
                    accepted = true;
                } else if (currentTemperature > minTemperature_) {
                    std::uniform_real_distribution<> dis(0.0, 1.0);
                    double probability = std::exp(-delta / currentTemperature);
                    if (dis(gen) < probability) {
                        accepted = true;
                    }
                }

                if (accepted) {
                    delete currentSolution;
                    currentSolution = newSolution;
                    currentCost = newCost;

                    if (currentCost < bestCost - 1e-9) {
                        bestCost = currentCost;
                        bestSolution->copy(currentSolution);
                        this->os << "New best: " << bestCost << std::endl;
                        notifyImprovement(bestSolution, bestCost);
                        iter = 1;
                    }
                } else {
                    delete newSolution;
                }

                currentTemperature *= coolingRate_;
                currentTemperature = std::max(currentTemperature, minTemperature_);

                segmentIter++;
                if (segmentIter >= segmentSize_) {
                    updateOperatorWeights();
                    segmentIter = 0;
                }
            }

            this->solution->copy(bestSolution);
            delete currentSolution;
            delete bestSolution;

            this->os << this->problem->getName() << "\t" << this->solution->getCost() << std::endl;
            this->solution->print(this->os);

            generator = previousGenerator;
            return this->solution != nullptr;
        }

    protected:
        void initializeOperators() {
            destroyOps.push_back(new RandomRemoval(gen, 0.2));
            destroyOps.push_back(new WorstRemoval(gen, 0.2, 0.1));
            destroyOps.push_back(new ShawRemoval(gen, 0.2));
            destroyOps.push_back(new RouteRemoval(gen, 1));

            repairOps.push_back(new GreedyRepair(gen));
            repairOps.push_back(new RegretRepair(gen, 2));
            repairOps.push_back(new RandomRepair(gen));
        }

        void destroyAndRepair(Solution* solution) {
            std::vector<double> destroyWeights(destroyOps.size());
            std::vector<double> repairWeights(repairOps.size());

            for (size_t i = 0; i < destroyOps.size(); ++i) {
                destroyWeights[i] = destroyOps[i]->weight;
            }
            for (size_t i = 0; i < repairOps.size(); ++i) {
                repairWeights[i] = repairOps[i]->weight;
            }

            std::discrete_distribution<size_t> destroyDist(destroyWeights.begin(), destroyWeights.end());
            std::discrete_distribution<size_t> repairDist(repairWeights.begin(), repairWeights.end());

            size_t destroyIdx = destroyDist(gen);
            size_t repairIdx = repairDist(gen);

            destroyOps[destroyIdx]->destruct(solution, problem);
            bool repairSuccess = repairOps[repairIdx]->repair(solution, problem);

            if (repairSuccess) {
                for (auto* neighbor : neighbors) {
                    neighbor->look(solution);
                }
            }
        }

        void updateOperatorWeights() {
            for (auto* destroy : destroyOps) {
                destroy->updateWeight(reactionFactor_, decayFactor_);
            }
            for (auto* repair : repairOps) {
                repair->updateWeight(reactionFactor_, decayFactor_);
            }
        }
    };
}
