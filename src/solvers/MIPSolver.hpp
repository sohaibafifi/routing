//
// Created by afifi on 01/02/18.
//

#ifndef HYBRID_MIPSOLVER_HPP
#define HYBRID_MIPSOLVER_HPP
#include <string>
#include <ilcplex/ilocplexi.h>
#include "../data/problem.hpp"
#include "Solver.hpp"
#include "../ConfigureCallbacksCall.h"
namespace routing{
template <class Reader>
class MIPSolver : public Solver<Reader>
{
    IloEnv env;
    IloCplex cplex;
    IloCplex model;
  public:
    MIPSolver(const std::string & p_inputFile, Config& config, std::ostream& os  = std::cout);
    virtual bool solve(double timeout = 3600) override;
    virtual ~MIPSolver() ;
};
}
template<class Reader>
routing::MIPSolver<Reader>::MIPSolver(const std::string & p_inputFile, Config& config, std::ostream& os):Solver<Reader> (p_inputFile,os)
{
    this->model = this->problem->generateModel(this->env);
    this->cplex = IloCplex(this->model);
    if(config.getActiveHeuristicCallback())
        this->cplex.use(this->problem->setHeuristicCallback(this->env));

    if(config.getActiveUserCutCallback())
        this->cplex.use(this->problem->setUserCutCallback(this->env));

    if(config.getActiveIncumbentCallback())
        this->cplex.use(this->problem->setIncumbentCallback(this->env));

    if(config.getActiveInformationCallback())
        this->cplex.use(this->problem->setInformationCallback(this->env));

    if(config.getActiveLazyConstraintCallback())
        this->cplex.use(this->problem->setLazyConstraintCallback(this->env));
}

template<class Reader>
bool routing::MIPSolver<Reader>::solve(double timeout)
{
    this->cplex.setParam(this->cplex.MIPEmphasis, this->cplex.MIPEmphasisFeasibility);
    this->cplex.setParam(this->cplex.Threads, 1);
    this->cplex.setParam(this->cplex.HeurFreq, 0);
    this->cplex.setParam(this->cplex.PreInd, 0);
    this->cplex.setParam(this->cplex.MIPDisplay, 1);
    this->cplex.setParam(this->cplex.TiLim, timeout);
    // this->model.exportModel(std::string(problem->getName() + ".lp").c_str());
    this->cplex.solve();
    this->os << this->problem->getName()
              << "\t" << this->cplex.getStatus( )
              << "\t" << this->cplex.getObjValue()
              << "\t" << this->cplex.getBestObjValue()
              << "\t" << this->cplex.getMIPRelativeGap()
              << "\t" << this->cplex.getTime()
              << std::endl;
}

template<class Reader>
routing::MIPSolver<Reader>::~MIPSolver()
{
    this->os << "deleting the problem from memory" << std::endl;
    delete this->problem;
    this->os << "deleting cplex" << std::endl;
    this->cplex.end();
    this->os << "deleting env" << std::endl;
    this->env.end();
    this->os  << "solver done" << std::endl;
}

#endif //HYBRID_MIPSOLVER_HPP
