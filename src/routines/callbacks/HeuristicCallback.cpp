#include "HeuristicCallback.hpp"
#include "../../Utility.hpp"
#include "../../mtrand.hpp"
#include "../neighborhoods/Move.hpp"
#include "../neighborhoods/IDCH.hpp"
#include "../../mtrand.hpp"

void routing::callback::HeuristicCallback::main()
{
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



    if (!hasIncumbent() || solution->getCost() < getIncumbentObjValue() - 1e-9) {
        IloNumVarArray vars(getEnv());
        IloNumArray vals(getEnv());
        solution->getVarsVals(vars, vals);

        for (unsigned i = 0; i < vars.getSize(); ++i) setBounds(vars[i], vals[i], vals[i]);

        solve();

        if(hasIncumbent())
            getEnv().out() << "CVRPHeuristicCallback from " << getIncumbentObjValue() << " to " << solution->getCost()
                       << " -  " << getObjValue() << "  " << getCplexStatus() << std::endl;
        InitialFound = (getCplexStatus() == CPX_STAT_OPTIMAL);
        for (unsigned i = 0; i < vars.getSize(); ++i)   vals[i] = getValue(vars[i]);
        setSolution(vars, vals, getObjValue());
        vals.end(); vars.end();
    }
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

