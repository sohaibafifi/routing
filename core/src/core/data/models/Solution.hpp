// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/Model.hpp"
#include "Tour.hpp"
#include "core/data/Problem.hpp"
#include <algorithm>

namespace routing {

    namespace models {

        struct Solution : public Model {
        protected:
            Problem *problem;

        public :
            Solution(Problem *p_problem) :
                    problem(p_problem),
                    notserved(std::vector<Client *>()) {
                for (unsigned i = 0; i < p_problem->clients.size(); ++i) {
                    notserved.push_back(p_problem->clients[i]);
                }
            }

            virtual ~Solution() {}


            virtual Solution * initFromSequence(Problem *problem, std::vector<Client*> sequence) = 0;
            virtual std::vector<Client*>  getSequence() = 0;

            Problem *getProblem() {
                return problem;
            }

            std::vector<Client *> notserved;

            virtual double getCost() = 0;

            virtual void update() = 0;
            
            virtual void pushTour(Tour *tour) = 0;

            virtual void addTour(Tour *tour, unsigned long position) = 0;

            virtual void overrideTour(Tour *tour, unsigned long position) = 0;

            virtual unsigned long getNbTour() const = 0;
#ifdef CPLEX_FOUND

            virtual void getVarsVals(IloNumVarArray &vars, IloNumArray &vals) = 0;

            virtual void constructFromModel(IloCplex::HeuristicCallbackI *pCallback) = 0;
#endif
            virtual void print(std::ostream &out) = 0;

            virtual models::Solution *clone() const {
                Solution *solution = this->problem->initializer()->initialSolution();
                *solution = *this;
                return solution;
            }

            virtual void copy(const models::Solution *) = 0;

            Solution &operator=(const Solution &p_solution) { // copy assignment constructor
                // protect against self assignment
                if (this != &p_solution) {
                    this->copy(&p_solution);
                }
                return *this;
            }

            virtual routing::models::Tour *getTour(unsigned t) const = 0;


            virtual void pushClient(unsigned long index_tour, Client *client) {
                this->notserved.erase(
                        std::remove(this->notserved.begin(), this->notserved.end(),
                                    client),
                        this->notserved.end());
                this->getTour(index_tour)->_pushClient(client);
            }

            virtual void addClient(unsigned long index_tour, Client *client, unsigned long position, routing::InsertionCost * cost) {
                assert(cost->isPossible());
                this->notserved.erase(
                        std::remove(this->notserved.begin(), this->notserved.end(),
                                    client),
                        this->notserved.end());
                this->getTour(index_tour)->addClient(client, position, cost);
            }

            virtual void removeClient(unsigned long index_tour, unsigned long position) {
                this->notserved.push_back(this->getTour(index_tour)->getClient(position));
                this->getTour(index_tour)->removeClient(position);
            }

        };

    }
}

