#include "HeuristicCallback.hpp"
#include "../../Utility.hpp"
#include "../../mtrand.hpp"
#include "../neighborhoods/Move.hpp"
#include "../neighborhoods/IDCH.hpp"
#include "../../mtrand.hpp"
#include "../../problems/cvrptw/routines/divers/Diver.hpp"
#include "../../problems/cvrptw/routines/Callbacks/HeuristicCallback.hpp"
#include "../../problems/cvrptw/solvers/Checker.hpp"

#include <unistd.h>

void routing::callback::HeuristicCallback::main()
{


    //double randomValue = static_cast<double>(rand())/RAND_MAX;

    double quota = getVariableQuotaToOne();

    if(quota <= Configuration::epsilon){ // if 0% of vars are set to one then construct a solution from scratch
        generator->generate(solution);
    }
    else if ( (quota - Configuration::maxQuotaDiving) <= Configuration::epsilon ){
        /*
         * Trying to dive
         */

        solution = extractPartialSolution(problem);

        routing::forbiddenPositions fp  = extractForbiddenPositions(problem);

        CVRPTW::Diver* diver = new CVRPTW::Diver();
        diver->dive(solution, &fp);
        /*CVRPTW::Checker* checker = new CVRPTW::Checker(solution,std::cout);
        checker->check();
        */

    }
    else{
        if (!hasIncumbent() ) {
            getEnv().out() << "Construct solution from scratch " << std::flush;
            if(! generator->generate(solution)){
                //no solution generator is defined
                std::cout <<  std::endl;
                return ;
            }
            std::cout << solution->getCost()<< std::endl;
        }
        else {
            if(neighbors.empty()) return;
            extractSolution();
            Utilities::MTRand_int32 irand(std::time(nullptr));
            bool improved = false;
            std::vector<bool> run(neighbors.size(), false);
            while(std::find(run.begin(), run.end(), false) != run.end()){
                unsigned i = 0;
                do{i = irand() % run.size();} while(run[i]);

                if(neighbors[i]->look(solution)){
                    improved = true;
                    run = std::vector<bool>(neighbors.size(), false);
                }else {
                    run[i] = true;
                }
            }
            if(!improved) return;
        }

        /*CVRPTW::Checker* checker = new CVRPTW::Checker(solution,std::cout);
        checker->check();
*/
    }

    //numberofDivings++;


    if (!hasIncumbent() || solution->getCost() < getIncumbentObjValue() - 1e-9) {
        IloNumVarArray vars(getEnv());
        IloNumArray vals(getEnv());
        solution->getVarsVals(vars, vals);

        for (unsigned i = 0; i < vars.getSize(); ++i) setBounds(vars[i], vals[i], vals[i]);

        /*CVRPTW::Checker* checker = new CVRPTW::Checker(solution,std::cout);
        checker->check();*/
        solve();

        if(hasIncumbent())
            getEnv().out() << "CVRPHeuristicCallback from " << getIncumbentObjValue() << " to " << solution->getCost()
                       << " -  " << getObjValue() << "  " << getCplexStatus() << std::endl;
        InitialFound = (getCplexStatus() == CPX_STAT_OPTIMAL);
        for (unsigned i = 0; i < vars.getSize(); ++i)   vals[i] = getValue(vars[i]);
        setSolution(vars, vals, getObjValue());
        vals.end(); vars.end();
    }
    // reset heuristic callback to pick up another IDCH randomly
    IloEnv env = getEnv();
    problem->setHeuristicCallback(env);
}


double routing::callback::HeuristicCallback::getVariableQuotaToOne() {
    int nbVarToOne = 0;
    int numberOfTours = 0;
    for(int i = 0; i < problem->arcs.size(); i++ ){
        for(int j = 0; j < problem->arcs.size(); j++ ){
            if(std::abs(getValue(problem->arcs[i][j])-1) < Configuration::epsilon){
                if(i == 0 ) numberOfTours++;
                nbVarToOne++;
            }
        }
    }

    double quota = (nbVarToOne *100.0 / (problem->clients.size() + numberOfTours))/100;
    return quota;

}

IloCplex::CallbackI *routing::callback::HeuristicCallback::duplicateCallback() const
{
    throw new std::logic_error("Not implemented");
}

void routing::callback::HeuristicCallback::extractSolution()
{
    throw new std::logic_error("Not implemented");
}

void routing::callback::HeuristicCallback::initSolution()
{
    throw new std::logic_error("Not implemented");
}

routing::models::Solution* routing::callback::HeuristicCallback::extractPartialSolution(routing::Problem* problem)
{
    throw new std::logic_error("Not implemented");
}

routing::forbiddenPositions routing::callback::HeuristicCallback::extractForbiddenPositions(routing::Problem *problem)
{
    throw new std::logic_error("Not implemented");
}



