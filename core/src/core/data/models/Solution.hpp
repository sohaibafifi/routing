#pragma once

#include "core/data/Model.hpp"
#include "Tour.hpp"
#include "core/data/Problem.hpp"

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

            Problem *getProblem() {
                return problem;
            }

            std::vector<Client *> notserved;

            virtual double getCost() = 0;


            virtual void pushTour(Tour *tour) = 0;

            virtual void addTour(Tour *tour, unsigned long position) = 0;

            virtual unsigned long getNbTour() const = 0;

            virtual void getVarsVals(IloNumVarArray &vars, IloNumArray &vals) = 0;

            virtual void print(std::ostream &out) = 0;

            virtual models::Solution *clone() const  {
                Solution *solution = this->problem->initializer()->initialSolution();
                *solution = *this;
                return solution;
            }

            virtual void copy(const models::Solution *)  = 0;

            Solution &operator=(const Solution &p_solution) { // copy assignment constructor
                // protect against self assignment
                if (this != &p_solution) {
                    this->copy(&p_solution);
                }
                return *this;
            }

            virtual routing::models::Tour *getTour(unsigned t) const = 0;


            virtual void pushClient(unsigned  long index_tour, Client *client){
                 this->notserved.erase(
                                std::remove(this->notserved.begin(), this->notserved.end(),
                                            client),
                                this->notserved.end());
                this->getTour(index_tour)->pushClient(client);
            }

            virtual void addClient(unsigned  long index_tour, Client *client, unsigned long position){
                this->notserved.erase(
                                std::remove(this->notserved.begin(), this->notserved.end(),
                                            client),
                                this->notserved.end());
                this->getTour(index_tour)->addClient(client, position);
            }

            virtual void removeClient(unsigned  long index_tour, unsigned long position) {
                this->notserved.push_back(this->getTour(index_tour)->getClient(position));
                this->getTour(index_tour)->removeClient(position);
            }

        };
        
    }
}

