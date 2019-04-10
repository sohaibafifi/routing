//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_SOLUTION_H
#define HYBRID_SOLUTION_H

#include "Problem.hpp"
#include "Visit.hpp"
#include "Tour.hpp"
#include "../../cvrp/models/Solution.hpp"

namespace CVRPTW{
    struct Solution : public CVRP::Solution{
    public:
        Solution(Problem * problem);

        virtual void updateMaxShifts();
        virtual void print(std::ostream &out) override;
        std::vector<Visit *> visits;
        virtual void update() override;
        virtual void addTour(routing::models::Tour *tour, unsigned long position) override;
        virtual unsigned long getNbTour() const  override { return tours.size();}

        virtual routing::models::Solution *clone() const override;
        virtual routing::models::Tour *getTour(unsigned t) override;

        virtual void updateTimeVariables(routing::models::Tour *tour, unsigned int position, double shift_j);
    protected:
        std::vector<Tour*> tours;
    };

}

#endif //HYBRID_SOLUTION_H
