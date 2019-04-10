//
// Created by ali on 3/28/19.
//

#include "HeuristicCallback.hpp"
#include "../operators/Constructor.hpp"
#include "../operators/Destructor.hpp"

routing::callback::HeuristicCallback *CVRPTW::Problem::setHeuristicCallback(IloEnv &env)
{
    std::vector<routing::Neighborhood*> neighborhoods;
    return new HeuristicCallback(env, this,
                                 new routing::Generator(new CVRPTW::Constructor, new CVRPTW::Destructor),
                                 neighborhoods);
}
