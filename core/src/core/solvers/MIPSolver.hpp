
// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "Solver.hpp"
#include "../routines/callbacks.hpp"
#include <ilcplex/ilocplex.h>

namespace routing {
    template<class Reader>
    class MIPSolver : public Solver<Reader> {
        IloCplex cplex;
    public:

        MIPSolver(const std::string &p_inputFile, std::ostream &os = std::cout);

        virtual void setDefaultConfiguration() override {
            this->configuration = new Configuration();
        };

        virtual bool solve(double timeout = 3600) override;
        virtual void tune(double timeout = 3600) ;


        virtual ~MIPSolver();

        virtual IloCplex getCplex() const { return cplex; }

        virtual void save(std::ofstream &output) const override;
    };
}

template<class Reader>
routing::MIPSolver<Reader>::MIPSolver(const std::string &p_inputFile, std::ostream &os):Solver<Reader>(
        p_inputFile) {
    this->cplex = this->problem->generateModel();

    this->setDefaultConfiguration();
    std::vector<IloCplex::CallbackI *> callbacks = std::vector<IloCplex::CallbackI *>();
    callbacks.push_back(this->problem->setHeuristicCallback());
    callbacks.push_back(this->problem->setUserCutCallback());
    callbacks.push_back(this->problem->setIncumbentCallback());
    callbacks.push_back(this->problem->setInformationCallback());
    callbacks.push_back(this->problem->setLazyConstraintCallback());
    for (auto callback: callbacks) {
        if (callback != nullptr)
            this->cplex.use(callback);
    }


}




template<class Reader>
bool routing::MIPSolver<Reader>::solve(double timeout) {

    // this->cplex.setParam(this->cplex.Threads, 1);
    this->cplex.setParam(IloCplex::Param::MIP::Display, 4);
    this->cplex.setParam(IloCplex::Param::MultiObjective::Display, 2);
    this->cplex.setParam(IloCplex::Param::TimeLimit, timeout);
    //this->cplex.setParam(IloCplex::Param::Preprocessing::Reduce, 0);
    cplex.resetTime();
    bool solved = this->cplex.solve() != 0;
    this->os << this->problem->getName()
             << "\t" << this->cplex.getStatus()
             << "\t" << this->cplex.getObjValue()
             << "\t" << this->cplex.getBestObjValue()
             << "\t" << this->cplex.getMIPRelativeGap()
             << "\t" << this->cplex.getTime()
             << std::endl;
    this->solution = this->problem->initializer()->initialSolution();
    // this->solution.forceCost();

    return solved;
}


template<class Reader>
void routing::MIPSolver<Reader>::tune(double timeout) {
    IloCplex::ParameterSet paramSet = cplex.getParameterSet();

    // this->cplex.setParam(this->cplex.Threads, 1);
    paramSet.setParam(IloCplex::Param::MIP::Display, 4);
    paramSet.setParam(IloCplex::Param::MultiObjective::Display, 2);
    paramSet.setParam(IloCplex::Param::TimeLimit, timeout);
    paramSet.setParam(IloCplex::Param::Tune::TimeLimit, timeout / 5.0);
    cplex.setParameterSet(paramSet);

    IloInt  tunestat = this->cplex.tuneParam(paramSet) ;
      if ( tunestat == IloCplex::TuningComplete)
         std::cout << "Tuning complete." << std::endl;
      else if ( tunestat == IloCplex::TuningAbort)
         std::cout << "Tuning abort." << std::endl;
      else if ( tunestat == IloCplex::TuningTimeLim)
         std::cout << "Tuning time limit." << std::endl;
      else
         std::cout << "Tuning status unknown." << std::endl;

}

template<class Reader>
routing::MIPSolver<Reader>::~MIPSolver() {
    delete this->problem;
    this->cplex.end();
}

template<class Reader>
void routing::MIPSolver<Reader>::save(std::ofstream &output) const {
    output <<
           this->getProblem()->getName()
           << "\t" << getCplex().getStatus()
           << "\t" << getCplex().getObjValue()
           << "\t" << getCplex().getBestObjValue()
           << "\t" << getCplex().getMIPRelativeGap()
           << "\t" << getCplex().getTime()
           << std::endl;
    output.close();
}