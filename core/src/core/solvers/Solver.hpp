//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once

#include "../data/models/Solution.hpp"
#include "../data/Problem.hpp"

template<class Reader>
class Solver {
    IloEnv env;
    IloCplex cplex;
    IloCplex model;
protected :
    routing::Problem *problem;
    routing::models::Solution *solution;

public:
    std::string inputFile;
    std::ostream &os;

    Solver(const std::string &p_inputFile, std::ostream &os = std::cout) : inputFile(p_inputFile), os(os) {
        this->problem = Reader().readFile(p_inputFile);

    }

    virtual bool solve(double timeout = 3600) = 0;

    routing::models::Solution *getSolution() const;
};

template<class Reader>
routing::models::Solution *Solver<Reader>::getSolution() const {
    return solution;
}
