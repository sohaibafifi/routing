#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ilcplex/ilocplexi.h>
#pragma GCC diagnostic pop
#include "../../data/problem.hpp"
#include "../../data/models/Solution.hpp"
namespace routing
{
	namespace callback
	{

		class InformationCallback
			: public IloCplex::MIPCallbackI
		{
		  public:
			InformationCallback(IloEnv env, Problem *_problem)
				:
				IloCplex::MIPCallbackI(env),
				problem(_problem)
			{}
			~InformationCallback()
			{}
		  protected:
			Problem *problem;
			void main();
			IloCplex::CallbackI *duplicateCallback() const;
            virtual models::Solution *getIncumbentSolution(){ return nullptr;}
		};
	}
}
