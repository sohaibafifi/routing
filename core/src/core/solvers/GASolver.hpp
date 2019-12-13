//
// Created by Sohaib LAFIFI on 22/11/2019.
//

#pragma once

#include "Solver.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/neighborhoods/Neighborhood.hpp"
#include <cassert>
#include <algorithm>
#include <set>
#include <utilities/Utilities.hpp>

namespace routing {
    class Sequence {

    public :
        Problem *problem;
        std::vector<models::Client *> sequence;

        Sequence(models::Solution *p_solution) : problem(p_solution->getProblem()) {
            sequence = p_solution->getSequence();
            hash = getHash();
            cost = p_solution->getCost();
        }

        Sequence(Problem *p_problem, std::vector<models::Client *> p_sequence) : problem(p_problem) {
            sequence = p_sequence;
            hash = getHash();
            cost = decode()->getCost();
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
        }

        models::Solution *decode() {
            models::Solution *solution = problem->initializer()->initialSolution();
            return solution->initFromSequence(problem, this->sequence);
        }

        bool updated = true;
        long hash = 0;
        double cost = 0;

        long getHash() {
            if (updated) {
                if (sequence.empty()) {
                    updated = false;
                    hash = 0;
                    return hash;
                }
                std::string sequence_str;
                for (auto &client : sequence) {
                    sequence_str.append(Utilities::itos(client->getID()));
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

        }

        GASolver(const std::string &p_inputFile,
                 std::ostream &os = std::cout) : Solver<Reader>(p_inputFile, os) {

        }

        virtual void setGenerator(Generator *p_generator) { this->generator = p_generator; }

        virtual void setNeighbors(std::vector<routing::Neighborhood *> p_neighbors) { this->neighbors = p_neighbors; }

        virtual void mutate(Sequence * sequence){

        }

        virtual bool solve(double timeout = 3600) override {
            assert(generator != nullptr);
            this->solution = this->problem->initializer()->initialSolution();
            Population *population = Population::initialize(this->problem);
            int itermax = this->problem->clients.size() * this->problem->clients.size();
            int iter = 1;
            routing::models::Solution *best = population->best()->decode()->clone();
            double bestCost = best->getCost();
            std::random_device rd;
            while (iter++ < itermax) {
                Sequence *child = population->evolve();
                // TODO : investigate mutation probability
                if( (rd() * 1.0 / rd.max() * 1.0) < (iter * 1.0 / itermax * 1.0)   )
                     mutate(child);
                if (population->insert(child)) iter = 1;
                if (population->best()->decode()->getCost() < bestCost - 1e-9) {
                    this->os << bestCost << std::endl;
                    best->copy(population->best()->decode());
                    bestCost = best->getCost();
                }
            }
            this->solution->copy(best);
            delete population;
            return this->solution != nullptr;
        }

        virtual ~GASolver() = default;
    };
}
