#pragma once
#include <sstream>

#include "../../../data/models/Solution.hpp"
#include "Client.hpp"
#include "Tour.hpp"

namespace CVRP
{
    struct Solution
    : public routing::models::Solution{
        public:
            Solution(Problem * p_problem):
                routing::models::Solution(p_problem),
                traveltime(0),
                tours(std::vector<Tour *>())
                {}
            Solution(routing::models::Solution & solution):
                routing::models::Solution(solution),
                traveltime(0){}
            virtual double getCost() override {return traveltime;}
            virtual void pushTour(routing::models::Tour *tour) override;
            virtual void addTour(routing::models::Tour *tour, unsigned long position) override;
            routing::Duration traveltime;
            virtual void getVarsVals(IloNumVarArray & vars, IloNumArray & vals) override;
            virtual unsigned long getNbTour() const  override { return tours.size();}


            virtual void print(std::ostream &out) override;

            virtual routing::models::Solution * clone() const override;
            virtual void update() override;

            virtual routing::models::Tour *getTour(unsigned t) override{return tours.at(t);}
        protected :
            std::vector<Tour*> tours;

    };
}
