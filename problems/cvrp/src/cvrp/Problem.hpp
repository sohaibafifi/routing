//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once

#include <vrp/Problem.hpp>

namespace cvrp {
    class Problem : public vrp::Problem {
    public :
        std::vector<IloNumVar> consumption;
    protected:
        routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;

        virtual void addCapacityConstraints();

        void addConstraints() override;

        void addVariables() override;
    };

}



