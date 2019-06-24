#pragma once
#include "../Model.hpp"
#include "Tour.hpp"
#include "../problem.hpp"
namespace routing {

    namespace models {

        struct Solution : public Model{
            protected:
                Problem * problem;
            public :
                Solution(Problem * p_problem);
                virtual ~Solution();
                Problem * getProblem();

                std::vector<Client *> notserved;
                std::vector<Client *> toRoute;
                virtual double getCost() = 0;
                virtual void update(routing::models::Solution* copy) = 0;
                virtual void pushTour(Tour* tour) = 0;
                virtual void addTour(Tour* tour, unsigned long position) = 0;
                virtual unsigned long getNbTour() const = 0;
                virtual void getVarsVals(IloNumVarArray & vars, IloNumArray & vals) = 0;
                virtual void print(std::ostream & out) = 0;
                virtual models::Solution * clone() const = 0;
                virtual routing::models::Tour * getTour(unsigned t) const = 0;
        };


    }
}

