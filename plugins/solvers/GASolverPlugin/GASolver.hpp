//
// Created by Sohaib LAFIFI on 22/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "plugins/solvers/SolverCorePlugin/Solver.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Generator.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"
#include "plugins/attributes/ComposableCorePlugin/Entity.hpp"
#include "core/interfaces/IEvaluator.hpp"
#include <cassert>
#include <algorithm>
#include <chrono>
#include <functional>
#include <set>
#include <limits>
#include <sstream>
#include <iomanip>
#include <utility>

namespace routing {
    enum class ConstraintMode {
        DistanceOnly,
        FeasibleOnly,
        Penalized
    };

    struct ConstraintConfig {
        ConstraintMode mode = ConstraintMode::DistanceOnly;
        double penaltyWeight = 1000.0;
        double unservedPenalty = 100000.0;
    };

    class Sequence {
    protected :
        bool updated = true;
        bool decoded = false;
        long hash = 0;
        routing::Duration cost = 0;
        Solution *solution = nullptr;
        ConstraintConfig config_;

    public :
        Problem *problem;
        std::vector<models::Client *> sequence;

        Sequence(Solution *p_solution, ConstraintConfig config = {})
            : problem(p_solution->getProblem()),
              sequence(p_solution->getSequence()),
              config_(config) {
            hash = getHash();
            cost = p_solution->getCost();
            problem->getMemory()->add(hash, cost);
        }

        Sequence(Problem *p_problem, const std::vector<models::Client *> & p_sequence,
                 ConstraintConfig config = {})
            : problem(p_problem),
              sequence(p_sequence),
              config_(config) {
            hash = getHash();
            cost = decode()->getCost();
            problem->getMemory()->add(hash, cost);
        }

        friend bool operator<(const Sequence &lhs, const Sequence &rhs) {
            if (lhs.cost != rhs.cost) {
                return lhs.cost < rhs.cost;
            }
            if (lhs.hash != rhs.hash) {
                return lhs.hash < rhs.hash;
            }
            return std::less<const Sequence*>{}(&lhs, &rhs);
        }

        Sequence(Problem *p_problem, ConstraintConfig config = {})
            : problem(p_problem),
              config_(config) {
            sequence = std::vector<models::Client *>();
            for (int i = 0; i < problem->clients.size(); ++i) {
                sequence.push_back(problem->clients.at(i));
            }
            std::random_device rd;
            std::shuffle(sequence.begin(), sequence.end(), rd);
            hash = getHash();
            cost = decode()->getCost();
            problem->getMemory()->add(hash, cost);
        }

        ~Sequence() {
            delete solution;
        }

        Solution *decode() {
            if (decoded && solution != nullptr) return solution;
            Solution *solution = problem->initializer()->initialSolution();
            this->solution = buildSolutionFromSequence(solution);
            this->cost = this->solution->getCost();
            decoded = true;
            return this->solution;
        }

        double getCost(){
            if(decoded) return cost;
            auto history_cost = problem->getMemory()->at(getHash());
            if(history_cost.first)
                return history_cost.second;
            return decode()->getCost();
        }

        long getHash() {
            if (updated) {
                if (sequence.empty()) {
                    updated = false;
                    hash = 0;
                    return hash;
                }
                std::ostringstream ss;
                ss << "mode=" << static_cast<int>(config_.mode)
                   << "|pen=" << std::fixed << std::setprecision(6) << config_.penaltyWeight
                   << "|unserved=" << config_.unservedPenalty << "|";
                for (models::Client * client : sequence) {
                    ss << client->getID() << '-';
                }
                std::hash<std::string> hash_fn_sequence;
                updated = false;
                hash = hash_fn_sequence(ss.str());
            }
            return hash;
        }

    private:
        Solution* buildSolutionFromSequence(Solution* base) {
            if (config_.mode == ConstraintMode::DistanceOnly) {
                return base->initFromSequence(problem, sequence);
            }

            base->setPenalty(0.0);
            base->notserved.clear();

            auto* depot = problem->getDepot();
            if (!depot) {
                return base->initFromSequence(problem, sequence);
            }

            const auto& evaluators = problem->getActiveEvaluators();
            if (evaluators.empty()) {
                return base->initFromSequence(problem, sequence);
            }

            auto vehicles = problem->getComposableVehicles();
            size_t numVehicles = vehicles.size();
            if (numVehicles == 0) {
                // No vehicles available - all clients must be unserved
                for (auto* client : sequence) {
                    base->notserved.push_back(client);
                }
                return base;
            }

            auto* depotEntity = dynamic_cast<Entity*>(depot);
            if (!depotEntity) {
                return base->initFromSequence(problem, sequence);
            }

            auto toEntity = [](models::Client* client) -> Entity* {
                return dynamic_cast<Entity*>(client);
            };

            auto buildContext = [&](Tour* currentTour, models::Client* client, size_t position)
                -> InsertionContext {
                size_t safePos = position;
                if (safePos > currentTour->getNbClient()) {
                    safePos = currentTour->getNbClient();
                }
                Entity* pred = depotEntity;
                Entity* succ = depotEntity;
                if (safePos > 0) {
                    pred = toEntity(currentTour->getClient(safePos - 1));
                }
                if (safePos < currentTour->getNbClient()) {
                    succ = toEntity(currentTour->getClient(safePos));
                }
                return InsertionContext{toEntity(client),
                                        static_cast<int>(safePos),
                                        pred,
                                        succ};
            };

            auto checkInsertion = [&](Tour* currentTour, models::Client* client, size_t position,
                                      int& failures) -> bool {
                failures = 0;
                auto ctx = buildContext(currentTour, client, position);
                if (!ctx.client || !ctx.predecessor || !ctx.successor) {
                    return true;
                }
                bool ok = true;
                for (auto* eval : evaluators) {
                    if (!eval->checkFeasibility(*currentTour, ctx)) {
                        ok = false;
                        failures++;
                    }
                }
                return ok;
            };

            double violationSum = 0.0;
            auto applyInsertion = [&](Tour* currentTour, models::Client* client, size_t position,
                                      int failures) {
                auto ctx = buildContext(currentTour, client, position);
                if (ctx.client && ctx.predecessor && ctx.successor) {
                    for (auto* eval : evaluators) {
                        eval->applyInsertion(*currentTour, ctx);
                    }
                }
                currentTour->_pushClient(client);
                if (failures > 0) {
                    violationSum += failures;
                }
            };

            size_t vehicleIdx = 0;
            auto* tour = new Tour(problem, static_cast<unsigned>(vehicleIdx));
            bool feasible = true;

            auto pushTour = [&]() {
                if (!tour) return;
                if (tour->getNbClient() > 0) {
                    base->pushTour(tour);
                } else {
                    delete tour;
                }
            };

            auto startNewTour = [&]() -> bool {
                if (vehicleIdx + 1 >= numVehicles) {
                    return false;
                }
                pushTour();
                vehicleIdx++;
                tour = new Tour(problem, static_cast<unsigned>(vehicleIdx));
                return true;
            };

            for (auto* client : sequence) {
                if (!tour) {
                    base->notserved.push_back(client);
                    feasible = false;
                    continue;
                }

                size_t position = tour->getNbClient();
                int failures = 0;
                bool ok = checkInsertion(tour, client, position, failures);

                if (!ok) {
                    bool started = startNewTour();
                    if (started) {
                        position = tour->getNbClient();
                        int newFailures = 0;
                        bool okNew = checkInsertion(tour, client, position, newFailures);
                        if (okNew) {
                            applyInsertion(tour, client, position, 0);
                            continue;
                        }
                        if (config_.mode == ConstraintMode::Penalized) {
                            applyInsertion(tour, client, position, newFailures);
                            continue;
                        }
                        base->notserved.push_back(client);
                        feasible = false;
                        continue;
                    }

                    if (config_.mode == ConstraintMode::Penalized) {
                        applyInsertion(tour, client, position, failures);
                        continue;
                    }

                    base->notserved.push_back(client);
                    feasible = false;
                    continue;
                }

                applyInsertion(tour, client, position, 0);
            }

            pushTour();
            base->update();

            if (config_.mode == ConstraintMode::Penalized) {
                double penalty = config_.penaltyWeight * violationSum;
                if (!base->notserved.empty()) {
                    penalty += config_.unservedPenalty * base->notserved.size();
                }
                base->setPenalty(penalty);
            } else if (!feasible || !base->notserved.empty()) {
                base->setPenalty(std::numeric_limits<double>::infinity());
            }

            return base;
        }
    };

    struct ChromosomeCmp {
        bool operator()(const Sequence *lhs, const Sequence *rhs) const {
            return *lhs < *rhs;
        }
    };

    class Population {
    public :
        Problem *problem;
        ConstraintConfig config;
        std::set<Sequence *, ChromosomeCmp> sequences;

        Population(Problem *p_problem, ConstraintConfig cfg)
            : problem(p_problem), config(cfg) {
            sequences = std::set<Sequence *, ChromosomeCmp>();
            while (sequences.size() < problem->clients.size()) {
                auto* sequence = new Sequence(problem, config);
                if (!sequences.insert(sequence).second) {
                    delete sequence;
                }
            }
        }

        static Population *initialize(Problem *p_problem, ConstraintConfig cfg) {
            Population *population = new Population(p_problem, cfg);
            return population;
        }

        Sequence *best() const {
            return *(this->sequences.begin());
        }

        virtual Sequence *evolve() {
            std::random_device rd;
            int i1 = rd() % sequences.size();
            int i2 = i1;
            while (i1 == i2) i2 = rd() % sequences.size();
            Sequence *parent1 = *std::next(sequences.begin(), i1);
            Sequence *parent2 = *std::next(sequences.begin(), i2);

            int delimiter1 = rd() % parent1->sequence.size();
            int delimiter2 = delimiter1;
            while (delimiter1 == delimiter2) delimiter2 = rd() % parent1->sequence.size();
            if (delimiter1 > delimiter2) std::swap(delimiter1, delimiter2);
            std::vector<bool> inserted(parent1->sequence.size() + 1, false);
            std::vector<models::Client *> child_sequence(parent1->sequence.size(), nullptr);
            for (int i = 0; i < parent1->sequence.size(); ++i) {
                if (i < delimiter1 || i >= delimiter2) {
                    child_sequence[i] = parent1->sequence.at(i);
                    inserted[parent1->sequence.at(i)->getID()] = true;
                }
            }
            for (int i = delimiter1; i < delimiter2; ++i) {
                for (int j = 0; j < parent2->sequence.size(); ++j) {
                    if (!inserted.at(parent2->sequence.at(j)->getID())
                        && child_sequence[i] == nullptr) {
                        child_sequence[i] = parent2->sequence.at(j);
                        inserted[parent2->sequence.at(j)->getID()] = true;
                    }
                }
            }
            Sequence *child = new Sequence(problem, child_sequence, config);
            return child;
        }

        bool insert(Sequence *sequence) {
            if (sequences.empty()) {
                return sequences.insert(sequence).second;
            }

            auto worst_it = std::prev(sequences.end());
            if (!(*sequence < **worst_it)) {
                return false;
            }

            if (!sequences.insert(sequence).second) {
                return false;
            }

            worst_it = std::prev(sequences.end());
            Sequence* to_remove = *worst_it;
            sequences.erase(worst_it);
            delete to_remove;
            return true;
        }

        ~Population() {
            for (auto* seq : sequences) {
                delete seq;
            }
            sequences.clear();
        }

    };

    class GASolver : public Solver {
    protected:
        std::vector<routing::Neighborhood *> neighbors;
        routing::Generator *generator = nullptr;
    public:
        GASolver(routing::Problem *p_problem,
                 Generator *p_generator,
                 const std::vector<Neighborhood *> &p_neighbors,
                 std::ostream &os = std::cout)
            : Solver(p_problem, os),
              generator(p_generator),
              neighbors(p_neighbors) {
            this->setDefaultConfiguration();
        }

        GASolver(routing::Problem *p_problem,
                 std::ostream &os = std::cout)
            : Solver(p_problem, os) {
            this->setDefaultConfiguration();
        }

        virtual void setGenerator(Generator *p_generator) { this->generator = p_generator; }

        virtual void setNeighbors(std::vector<routing::Neighborhood *> p_neighbors) { this->neighbors = p_neighbors; }

        virtual void mutate(Sequence *sequence) {

        }

        virtual void setDefaultConfiguration() override {
            this->configuration = new Configuration();
            this->configuration->setIntParam(this->configuration->iterMax,
                                             this->problem->clients.size() * this->problem->clients.size());
            this->configuration->setBoolParam("feasibleOnly", true);  // Enforce constraints strictly
            this->configuration->setDoubleParam("infeasiblePenalty", 1000.0);
            this->configuration->setDoubleParam("unservedPenalty", 100000.0);
        };

        virtual bool solve(double timeout = 3600) override {
            // Note: generator is not used in current implementation
            this->solution = this->problem->initializer()->initialSolution();
            ConstraintConfig config = buildConstraintConfig();
            Population *population = Population::initialize(this->problem, config);
            int itermax = this->configuration->getIntParam(this->configuration->iterMax);
            int iter = 1;
            double bestCost = population->best()->getCost();
            std::random_device rd;
            const auto start = std::chrono::steady_clock::now();
            while (iter++ < itermax) {
                if (timeout > 0) {
                    const auto elapsed = std::chrono::duration<double>(
                        std::chrono::steady_clock::now() - start).count();
                    if (elapsed >= timeout) {
                        break;
                    }
                }

                Sequence *child = population->evolve();
                // TODO : investigate mutation probability
                if( (rd() * 1.0 / rd.max() * 1.0) < (iter * 1.0 / itermax * 1.0)   )
                     mutate(child);
                if (population->insert(child)) {
                    iter = 1;
                } else {
                    delete child;
                }
                if (population->best()->getCost() < bestCost - 1e-9) {
                    this->os << bestCost << std::endl;
                    bestCost = population->best()->getCost();
                    // Notify callback of improvement
                    notifyImprovement(population->best()->decode(), bestCost);
                }
            }
            this->solution->copy(population->best()->decode());
            delete population;
            auto memory = this->problem->getMemory();
            return this->solution != nullptr;
        }

        virtual ~GASolver() = default;

    private:
        ConstraintConfig buildConstraintConfig() const {
            ConstraintConfig cfg;
            bool feasibleOnly = false;
            double penalty = cfg.penaltyWeight;
            double unserved = cfg.unservedPenalty;

            if (configuration) {
                try {
                    feasibleOnly = configuration->getBoolParam("feasibleOnly");
                } catch (const ParameterNotFound&) {
                }
                try {
                    penalty = configuration->getDoubleParam("infeasiblePenalty");
                } catch (const ParameterNotFound&) {
                }
                try {
                    unserved = configuration->getDoubleParam("unservedPenalty");
                } catch (const ParameterNotFound&) {
                }
            }

            cfg.penaltyWeight = penalty;
            cfg.unservedPenalty = unserved;

            if (feasibleOnly) {
                cfg.mode = ConstraintMode::FeasibleOnly;
            } else if (penalty > 0.0 || unserved > 0.0) {
                cfg.mode = ConstraintMode::Penalized;
            } else {
                cfg.mode = ConstraintMode::DistanceOnly;
            }

            return cfg;
        }
    };
}
