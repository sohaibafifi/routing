//
// Created by afifi on 01/02/18.
//

#ifndef HYBRID_SOLVER_HPP
#define HYBRID_SOLVER_HPP
#include <string>
#include <ilcplex/ilocplexi.h>
#include "data/problem.hpp"
class Solver
{
	IloEnv env;
	IloCplex cplex;
    IloCplex model;
    routing::Problem * problem;
  public:
	template <class Reader>
	static Solver*  create (std::string inputFile, double timeout = 3600);

	void solve();
	virtual ~Solver();

};

template<class Reader>
Solver *Solver::create(std::string inputFile, double timeout)
{
	Solver * solver = new Solver();
	solver->problem = Reader().readFile(inputFile);
    solver->model = solver->problem->generateModel(solver->env);
    solver->cplex = IloCplex(solver->model);
	solver->cplex.use(solver->problem->setHeuristicCallback(solver->env));
	solver->cplex.use(solver->problem->setUserCutCallback(solver->env));
	solver->cplex.use(solver->problem->setIncumbentCallback(solver->env));
	solver->cplex.use(solver->problem->setInformationCallback(solver->env));
	//solver->cplex.use(solver->problem->setLazyConstraintCallback(solver->env));
	solver->cplex.setParam(solver->cplex.TiLim, timeout);
	solver->cplex.setParam(solver->cplex.MIPEmphasis, solver->cplex.MIPEmphasisFeasibility);
	solver->cplex.setParam(solver->cplex.Threads, 1);
    solver->cplex.setParam(solver->cplex.HeurFreq, 0);
    solver->cplex.setParam(solver->cplex.PreInd, 0);
    solver->cplex.setParam(solver->cplex.MIPDisplay, 1);
    return solver;
}
void Solver::solve()
{
    // this->model.exportModel(std::string(problem->getName() + ".lp").c_str());
	this->cplex.solve();
	std::cout << problem->getName()
			  << "\t" << this->cplex.getStatus( )
			  << "\t" << this->cplex.getObjValue()
			  << "\t" << this->cplex.getBestObjValue()
			  << "\t" << this->cplex.getMIPRelativeGap()
			  << "\t" << this->cplex.getTime()
			  << std::endl;
}

Solver::~Solver()
{
	std::cout << "deleting the problem from memory" << std::endl;
	delete this->problem;
	std::cout << "deleting cplex" << std::endl;
	this->cplex.end();
	std::cout << "deleting env" << std::endl;
	this->env.end();
	std::cout << "solver done" << std::endl;
}

#endif //HYBRID_SOLVER_HPP
