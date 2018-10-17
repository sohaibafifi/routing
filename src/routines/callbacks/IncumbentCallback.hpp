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
		/*!
		 * @brief callback trigger every time a new incumbent is found
		 */
		class IncumbentCallback
			: public IloCplex::IncumbentCallbackI
		{
		  public:
			IncumbentCallback(IloEnv env, Problem *_problem)
				:
				IloCplex::IncumbentCallbackI(env),
				problem(_problem)
			{}
			~IncumbentCallback()
			{}
		  protected:
			Problem *problem;
            virtual void main();
            models::Solution * solution;
			IloCplex::CallbackI *duplicateCallback() const;
            virtual void extractIncumbentSolution(){}
		};
	}
}
