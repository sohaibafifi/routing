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

            virtual void update(routing::models::Solution *copy) = 0;

            virtual void pushTour(Tour *tour) = 0;

            virtual void addTour(Tour *tour, unsigned long position) = 0;

            virtual unsigned long getNbTour() const = 0;

            virtual void getVarsVals(IloNumVarArray &vars, IloNumArray &vals) = 0;

            virtual void print(std::ostream &out) = 0;

            virtual models::Solution *clone() const = 0;

            virtual routing::models::Tour *getTour(unsigned t) const = 0;
        };


    }
}

