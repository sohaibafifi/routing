#include "InformationCallback.hpp"
#include "../../Utility.hpp"
#include "../../mtrand.hpp"
IloCplex::CallbackI *routing::callback::InformationCallback::duplicateCallback() const
{

}

void routing::callback::InformationCallback::main()
{
    if(long(getDetTime()) % 1000 > 0 ) return;
	if(hasIncumbent())
		getEnv().out() << "I : " << getIncumbentObjValue() << "\t" << getBestObjValue() << "\t" << getMIPRelativeGap() << "\t" << getDetTime() << std::endl;
	else
		getEnv().out() << "I : " << '-' << "\t" << getBestObjValue() << "\t" << getMIPRelativeGap() << "\t" << getDetTime() << std::endl;

}

