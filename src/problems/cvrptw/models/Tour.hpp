//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_TOUR_H
#define HYBRID_TOUR_H

#include "../../cvrp/models/Tour.hpp"
#include "Problem.hpp"
#include "Visit.hpp"
#include "InsertionCost.hpp"
namespace CVRPTW{
    struct Tour : public CVRP::Tour{

    public:
        Tour(Problem * p_problem, unsigned vehicleID):
                CVRP::Tour(p_problem, vehicleID){}
        virtual routing::InsertionCost* evaluateInsertion(routing::models::Client *client, unsigned long position, std::vector<Visit*> visits);
        virtual routing::Duration evaluateRemove(unsigned long position) override;
        virtual void removeClient(unsigned long position) override;
        virtual void addClient(routing::models::Client *client, unsigned long position) override;

    };
}

#endif //HYBRID_TOUR_H
