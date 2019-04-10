//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_HEURISTICCALLBACK_H
#define HYBRID_HEURISTICCALLBACK_H

#include "../../../cvrp/routines/HeuristicCallback.hpp"
#include "../../models/Problem.hpp"
#include "../../models/Solution.hpp"

namespace CVRPTW {
    class HeuristicCallback
            : public CVRP::HeuristicCallback
    {
    public :
        HeuristicCallback(IloEnv env, Problem *_problem,
                          routing::Generator * p_generator,
                          std::vector<routing::Neighborhood*> p_neighbors)
                : CVRP::HeuristicCallback(env, _problem, p_generator, p_neighbors),
                  problem(_problem)
        {

        }
        Problem * problem; // avoid static_cast<Problem*>(problem)
        virtual void initSolution() override {
            solution = new Solution(problem);
        }
    };
}


#endif //HYBRID_HEURISTICCALLBACK_H
