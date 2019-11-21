//
// Created by Sohaib LAFIFI on 20/11/2019.
//


#include "Problem.hpp"
#include "models/Client.hpp"
#include "models/Vehicle.hpp"
#include <utilities/Utilities.hpp>
#include <core/routines/callbacks.hpp>
#include <vrp/routines/operators/Generator.hpp>

void cvrp::Problem::addConstraints() {
    vrp::Problem::addConstraints();
    this->addCapacityConstraints();
}

void cvrp::Problem::addCapacityConstraints() {
    for (auto k = 0; k < vehicles.size(); ++k) {
        IloExpr expr(model.getEnv());
        for (auto i = 1; i <= clients.size(); ++i) {
            expr += static_cast<models::Client *>(clients[i - 1])->getDemand() * affectation[i][k];
        }
        model.add(expr <= static_cast<models::Vehicle *>(vehicles[k])->getCapacity());
    }
    for (auto i = 1; i <= clients.size(); ++i) {
        for (auto j = 1; j <= clients.size(); ++j) {
            if (i == j) continue;
            model.add(consumption[i]
                      + static_cast<models::Client *>(clients[j - 1])->getDemand()
                      <= consumption[j]
                         + (static_cast<models::Vehicle *>(vehicles[0])->getCapacity()
                            + static_cast<models::Client *>(clients[j - 1])->getDemand()) * (1 - arcs[i][j]));
        }
    }
}

void cvrp::Problem::addVariables() {
    vrp::Problem::addVariables();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        if (i == 0)
            consumption.push_back(IloNumVar(model.getEnv(),
                                            0, 0,
                                            std::string("q_" + Utilities::itos(i)).c_str()));
        else

            consumption.push_back(IloNumVar(model.getEnv(),
                                            static_cast<models::Client *>(clients[i - 1])->getDemand(),
                                            static_cast<models::Vehicle *>(vehicles[0])->getCapacity(),
                                            std::string("q_" + Utilities::itos(i)).c_str()));
        model.add(consumption.back());
    }
}

routing::callback::HeuristicCallback *cvrp::Problem::setHeuristicCallback(IloEnv &env) {
    std::vector<routing::Neighborhood *> dummyNeighborhoods;
    return new routing::callback::HeuristicCallback(env,
                                                    this,
                                                    new vrp::routines::Generator(this, new routing::dummyConstructor(),
                                                                                 new routing::dummyDestructor()),
                                                    new routing::dummyDiver(),
                                                    dummyNeighborhoods);

}
