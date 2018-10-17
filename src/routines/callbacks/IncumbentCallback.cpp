#include "IncumbentCallback.hpp"
#include "../../Utility.hpp"
#include "../../mtrand.hpp"


void routing::callback::IncumbentCallback::main()
{
	getEnv().out() << "Incumbent found of value " << getObjValue() << std::endl;
    extractIncumbentSolution();
    solution->print(getEnv().out());
}

IloCplex::CallbackI *routing::callback::IncumbentCallback::duplicateCallback() const
{

}
