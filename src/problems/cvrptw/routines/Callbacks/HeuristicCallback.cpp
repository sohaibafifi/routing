//
// Created by ali on 3/28/19.
//

#include "HeuristicCallback.h"
#include "../operators/Constructor.h"
#include "../operators/Destructor.h"

routing::callback::HeuristicCallback *CVRPTW::Problem::setHeuristicCallback(IloEnv &env)
{
    std::vector<routing::Neighborhood*> neighborhoods;
    return new HeuristicCallback(env, this,
                                 new routing::Generator(new CVRPTW::Constructor, new CVRPTW::Destructor),
                                 neighborhoods);
}
