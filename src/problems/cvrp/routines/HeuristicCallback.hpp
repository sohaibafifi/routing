#pragma once
#include "../Problem.hpp"
#include "../models/Solution.hpp"
#include "../../../routines/callbacks/HeuristicCallback.hpp"


namespace CVRP
{

    class HeuristicCallback
            : public routing::callback::HeuristicCallback
    {
        public :
            HeuristicCallback(IloEnv env, Problem *_problem,
                              routing::Generator * p_generator,
                              std::vector<routing::Neighborhood*> p_neighbors)
                : routing::callback::HeuristicCallback(env, _problem, p_generator, p_neighbors),
                  problem(_problem)
            {

            }
        protected:
            Problem * problem; // avoid static_cast<Problem*>(problem)

            virtual void extractSolution() override;

            virtual void main() override;
            virtual void initSolution() override {
                solution = new Solution(problem);
            }
    };
}
