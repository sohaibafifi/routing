//
// Created by afifi on 01/02/18.
//

#ifndef HYBRID_SOLVER_HPP
#define HYBRID_SOLVER_HPP
#include <string>
#include "../data/problem.hpp"
#include "../data/models/Solution.hpp"
template <class Reader>
class Solver
{
	IloEnv env;
	IloCplex cplex;
    IloCplex model;
  protected :
    routing::Problem * problem;
    routing::models::Solution * solution;
  public:
    std::string inputFile;
    Solver(const std::string & p_inputFile):inputFile(p_inputFile){
        this->problem = Reader().readFile(p_inputFile);
    }
    virtual bool solve(double timeout = 3600) = 0;
    routing::models::Solution *getSolution() const;
};
template <class Reader>
routing::models::Solution *Solver<Reader>::getSolution() const
{
    return solution;
}
#endif //HYBRID_SOLVER_HPP
