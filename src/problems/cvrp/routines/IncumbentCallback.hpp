#pragma once
#include <sstream>
#include "../Problem.hpp"
#include "../models/Solution.hpp"
#include "../../../routines/callbacks/IncumbentCallback.hpp"
namespace CVRP
{
    class IncumbentCallback
            : public routing::callback::IncumbentCallback
    {
        public :
            IncumbentCallback(IloEnv env, Problem *_problem)
                : routing::callback::IncumbentCallback(env, _problem)
            {
                solution = new Solution(_problem);
            }
        protected:
            virtual void extractIncumbentSolution() override;
    };


}
