//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once

#include "Solver.hpp"
#include "../routines/callbacks.hpp"

namespace routing {
    template<class Reader>
    class MIPSolver : public Solver<Reader> {
        IloEnv env;
        IloCplex cplex;
        IloModel model;
    public:
        MIPSolver(const std::string &p_inputFile, std::ostream &os = std::cout);

        virtual bool solve(double timeout = 3600) override;

        virtual ~MIPSolver();
    };
}

template<class Reader>
routing::MIPSolver<Reader>::MIPSolver(const std::string &p_inputFile, std::ostream &os):Solver<Reader>(
        p_inputFile) {
    this->model = this->problem->generateModel(this->env);
    this->cplex = IloCplex(this->model);
    this->cplex.exportModel("model.lp");
    std::vector<IloCplex::CallbackI *> callbacks = std::vector<IloCplex::CallbackI *>();
    callbacks.push_back(this->problem->setHeuristicCallback(this->env));
    callbacks.push_back(this->problem->setUserCutCallback(this->env));
    callbacks.push_back(this->problem->setIncumbentCallback(this->env));
    callbacks.push_back(this->problem->setInformationCallback(this->env));
    callbacks.push_back(this->problem->setLazyConstraintCallback(this->env));
    for (auto callback : callbacks) {
        if (callback != nullptr)
            this->cplex.use(callback);
    }

}

template<class Reader>
bool routing::MIPSolver<Reader>::solve(double timeout) {
    this->cplex.setParam(this->cplex.MIPEmphasis, this->cplex.MIPEmphasisFeasibility);
    this->cplex.setParam(this->cplex.Threads, 1);
    this->cplex.setParam(this->cplex.HeurFreq, 0);
    this->cplex.setParam(this->cplex.PreInd, 0);
    this->cplex.setParam(this->cplex.MIPDisplay, 4);
    this->cplex.setParam(this->cplex.TiLim, timeout);
    bool solved = this->cplex.solve() != 0;
    this->os << this->problem->getName()
             << "\t" << this->cplex.getStatus()
             << "\t" << this->cplex.getObjValue()
             << "\t" << this->cplex.getBestObjValue()
             << "\t" << this->cplex.getMIPRelativeGap()
             << "\t" << this->cplex.getTime()
             << std::endl;
    return solved;
}

template<class Reader>
routing::MIPSolver<Reader>::~MIPSolver() {
    this->os << "deleting the problem from memory" << std::endl;
    delete this->problem;
    this->os << "deleting cplex" << std::endl;
    this->cplex.end();
    this->os << "deleting env" << std::endl;
    this->env.end();
    this->os << "solver done" << std::endl;
}

