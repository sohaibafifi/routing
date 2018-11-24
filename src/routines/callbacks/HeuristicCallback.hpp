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
                              routing::Generator *p_generator,
                              std::vector<routing::Neighborhood*> p_neighbors)
				:
                IloCplex::HeuristicCallbackI(env),
                problem(_problem),
                generator(p_generator),
                neighbors(p_neighbors)
            {
                InitialFound = false;
            }
			~HeuristicCallback() {}
		  protected:
            Problem *problem;
            routing::Generator *generator;
            std::vector<routing::Neighborhood*> neighbors;

            virtual void main();
            IloCplex::CallbackI *duplicateCallback() const;
            models::Solution * solution;
            virtual void extractSolution();
            bool InitialFound;
            virtual void initSolution();

        };
	}
}
