#pragma once
#include "../Problem.hpp"
#include "../models/Solution.hpp"
#include "../../../routines/callbacks/HeuristicCallback.hpp"

#include "operators/Constructor.hpp"
#include "operators/Destructor.hpp"
namespace CVRP
{

    class HeuristicCallback
            : public routing::callback::HeuristicCallback
    {
        public :
            HeuristicCallback(IloEnv env, Problem *_problem,
                              routing::Constructor * p_constructor,
                              routing::Destructor * p_destructor)
                : routing::callback::HeuristicCallback(env, _problem, p_constructor, p_destructor),
                  problem(_problem)
            {

            }
        protected:
            Problem * problem; // avoid static_cast<Problem*>(problem)

            virtual void extractSolution() override;

            virtual void main() override;
            virtual void initSolution() override {
                std::cout << "CVRP :: initSolution " << std::endl;
                solution = new Solution(problem);
            }
    };
}
