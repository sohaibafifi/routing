#include "HeuristicCallback.hpp"
#include "../../Utility.hpp"
#include "../../mtrand.hpp"
#include "../neighborhoods/IDCH.hpp"
void routing::callback::HeuristicCallback::main()
{
    if (!hasIncumbent() ) {
        getEnv().out() << "Construct solution from scratch " << std::flush;
        if(! Generator(constructor, destructor).generate(solution)){
            //no solution generator is defined
            std::cout <<  std::endl;
            return ;
        }
        std::cout << solution->getCost()<< std::endl;
    }
    else {
        extractSolution();
        IDCH idch(constructor, destructor);
        if(!idch.look(solution)) return;
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
        if(getCplexStatus() == CPX_STAT_OPTIMAL) InitialFound = true;
        for (unsigned i = 0; i < vars.getSize(); ++i)   vals[i] = getValue(vars[i]);
        setSolution(vars, vals, getObjValue());
        vals.end(); vars.end();
    }
}

IloCplex::CallbackI *routing::callback::HeuristicCallback::duplicateCallback() const
{

}

void routing::callback::HeuristicCallback::extractSolution()
{

}

void routing::callback::HeuristicCallback::initSolution()
{

}

