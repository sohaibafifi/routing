#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ilcplex/ilocplexi.h>
#include <map>

#pragma GCC diagnostic pop
#include "../../data/problem.hpp"
#include "../neighborhoods/Generator.hpp"
#include "../operators/Constructor.hpp"
#include "../operators/Destructor.hpp"
namespace routing
{
	//associate for each ClientID (i) the next ClientIDs (j) that can't follow it either
	//because x_(ij) = 0 or there exists a ClientID (k) such that x_(ik) = 1
	typedef std::map<unsigned int, std::vector<unsigned int>> forbiddenPositions;

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
			virtual routing::models::Solution* extractPartialSolution(routing::Problem* problem);
			virtual routing::forbiddenPositions extractForbiddenPositions(routing::Problem* problem);
            double getVariableQuotaToOne();
			bool InitialFound;
            virtual void initSolution();

        };
	}
}
