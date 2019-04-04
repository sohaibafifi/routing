//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_PROBLEM_H
#define HYBRID_PROBLEM_H

#include "../../cvrp/Problem.hpp"
#include "Depot.h"
#include "Client.h"

namespace CVRPTW{
    class Problem
            : public CVRP::Problem
    {
    public :
        friend class Reader;
        virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;
    protected :
        virtual void addVariables() override ;
        virtual void addSequenceConstraints() override ;
        std::vector<IloNumVar> start;
    };
}


#endif //HYBRID_PROBLEM_H
