#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ilcplex/ilocplexi.h>
#pragma GCC diagnostic pop
#include "../../data/problem.hpp"
#include "../neighborhoods/Generator.hpp"
#include "../operators/Constructor.hpp"
#include "../operators/Destructor.hpp"
namespace routing
{
	namespace callback
    {
		class HeuristicCallback
            : public IloCplex::HeuristicCallbackI
		{
		  public:
            HeuristicCallback(IloEnv env, Problem *_problem,
                              Constructor * p_constructor,
                              Destructor * p_destructor)
				:
                IloCplex::HeuristicCallbackI(env),
                problem((_problem)),
                constructor(p_constructor),
                destructor(p_destructor)
			{
                InitialFound = false;
            }
			~HeuristicCallback() {}
		  protected:
            Problem *problem;
            virtual void main();
            IloCplex::CallbackI *duplicateCallback() const;
            Constructor * constructor;
            Destructor * destructor;
            models::Solution * solution;
            virtual void extractSolution();
            bool InitialFound;
            virtual void initSolution();

        };
	}
}
