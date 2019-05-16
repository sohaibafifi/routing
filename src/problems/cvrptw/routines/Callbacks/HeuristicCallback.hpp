//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_HEURISTICCALLBACK_H
#define HYBRID_HEURISTICCALLBACK_H

#include "../../../cvrp/routines/HeuristicCallback.hpp"
#include "../../models/Problem.hpp"
#include "../../models/Solution.hpp"
#include "../../../../Utility.hpp"
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
        virtual void main() override;
        virtual void extractSolution() override;
        virtual routing::models::Solution* extractPartialSolution(routing::Problem* problem) override;
        virtual routing::forbiddenPositions extractForbiddenPositions(routing::Problem* problem) override;

        virtual void initSolution() override {
            solution = new Solution(problem);
        }
    };
}


#endif //HYBRID_HEURISTICCALLBACK_H
