//
// Created by Sohaib LAFIFI on 22/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "Solver.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/neighborhoods/Neighborhood.hpp"
#include <cassert>
#include <algorithm>
#include <set>
#include <utility>
#include "../data/attributes/GeoNode.hpp"
#include "../../utilities/Utilities.hpp"

namespace routing {
    class Sequence {
    protected :
        bool updated = true;
        bool decoded = false;
        long hash = 0;
        routing::Duration cost = 0;
        models::Solution *solution = nullptr;

    public :
        Problem *problem;
        std::vector<models::Client *> sequence;

        Sequence(models::Solution *p_solution) : problem(p_solution->getProblem()),
                                                sequence(p_solution->getSequence()){
            hash = getHash();
            cost = p_solution->getCost();
            problem->getMemory()->add(hash, cost);
        }

        Sequence(Problem *p_problem, const std::vector<models::Client *> & p_sequence) : problem(p_problem),
                                                                                         sequence(p_sequence) {
            hash = getHash();
            cost = decode()->getCost();
            problem->getMemory()->add(hash, cost);
        }

        friend bool operator<(const Sequence &lhs, const Sequence &rhs) {
            return rhs.cost > lhs.cost;
        }

        Sequence(Problem *p_problem) : problem(p_problem) {
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

        models::Solution *decode() {
            if (decoded && solution != nullptr) return solution;
            models::Solution *solution = problem->initializer()->initialSolution();
            this->solution = solution->initFromSequence(problem, this->sequence);
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
                std::string sequence_str;
                for (models::Client * client : sequence) {
                    sequence_str.append(std::to_string(client->getID()));
                    sequence_str.push_back('-');
                }
                std::hash<std::string> hash_fn_sequence;
                updated = false;
                hash = hash_fn_sequence(sequence_str);
            }
            return hash;
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
        std::set<Sequence *, ChromosomeCmp> sequences;

        Population(Problem *p_problem) : problem(p_problem) {
            sequences = std::set<Sequence *, ChromosomeCmp>();
            while (sequences.size() < problem->clients.size()) {
                sequences.insert(new Sequence(problem));
            }
        }

        static Population *initialize(Problem *p_problem) {
            Population *population = new Population(p_problem);
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
            Sequence *child = new Sequence(problem, child_sequence);
            return child;
        }

        bool insert(Sequence *sequence) {
            if (*sequence < **sequences.rbegin()) {
                if (sequences.insert(sequence).second) {
                    sequences.erase(prev(sequences.end()));
                    return true;
                } else return false;
            }
            return false;
        }

        ~Population() {
            while (std::begin(sequences) != std::end(sequences)) {
                std::set<Sequence *>::iterator to_delete = sequences.begin();
                sequences.erase(to_delete);
                delete *to_delete;
            }
        }

    };

    template<class Reader>
    class GASolver : public Solver<Reader> {
    protected:
        std::vector<routing::Neighborhood *> neighbors;
        routing::Generator *generator = nullptr;
    public:
        GASolver(const std::string &p_inputFile,
                 Generator *p_generator,
                 const std::vector<Neighborhood *> &p_neighbors,
                 std::ostream &os = std::cout) : Solver<Reader>(p_inputFile, os),
                                                 generator(p_generator),
                                                 neighbors(p_neighbors) {
            this->setDefaultConfiguration();

        }

        GASolver(const std::string &p_inputFile,
                 std::ostream &os = std::cout) : Solver<Reader>(p_inputFile, os) {
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
        };

        virtual bool solve(double timeout = 3600) override {
            assert(generator != nullptr);
            this->solution = this->problem->initializer()->initialSolution();
            Population *population = Population::initialize(this->problem);
            int itermax = this->configuration->getIntParam(this->configuration->iterMax);
            int iter = 1;
            double bestCost = population->best()->getCost();
            std::random_device rd;
            while (iter++ < itermax) {
                Sequence *child = population->evolve();
                // TODO : investigate mutation probability
                if( (rd() * 1.0 / rd.max() * 1.0) < (iter * 1.0 / itermax * 1.0)   )
                     mutate(child);
                if (population->insert(child)) iter = 1;
                if (population->best()->getCost() < bestCost - 1e-9) {
                    this->os << bestCost << std::endl;
                    bestCost = population->best()->getCost();
                }
            }
            this->solution->copy(population->best()->decode());
            delete population;
            auto memory = this->problem->getMemory();
            return this->solution != nullptr;
        }

        virtual ~GASolver() = default;
    };
}
