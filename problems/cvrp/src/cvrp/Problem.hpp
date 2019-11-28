//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once

#include <vrp/Problem.hpp>

namespace cvrp {
    class Problem : public vrp::Problem {
    protected:

        virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override {
            return nullptr;
        }

        void addConstraints() override {
            vrp::Problem::addConstraints();
            this->addCapacityConstraints();
        }

    };

}



