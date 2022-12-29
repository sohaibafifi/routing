// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"

routing::Initializer *cvrp::Problem::initializer() {
    return new cvrp::Initializer(this);
}

void cvrp::Problem::addConstraints() {
    vrp::Problem::addConstraints();
    this->addCapacityConstraints();
}

void cvrp::Problem::addCapacityConstraints() {
    for (auto k = 0; k < vehicles.size(); ++k) {
        IloExpr expr(env);
        for (auto i = 1; i <= clients.size(); ++i) {
            if (routing::attributes::Consumer *client = dynamic_cast<routing::attributes::Consumer *>(clients[i - 1]))
                expr += client->getDemand() * affectation[i][k];
        }
        if (routing::attributes::Stock *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[k]))
            model.add(expr <= stock->getCapacity());
    }
    if (auto *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[0]))
        for (auto i = 1; i <= clients.size(); ++i) {
            for (auto j = 1; j <= clients.size(); ++j) {
                if (i == j) continue;
                if (auto *client = dynamic_cast<routing::attributes::Consumer *>(clients[j - 1]))
                    model.add(consumption[i]
                              + client->getDemand()
                              <= consumption[j]
                                 + (stock->getCapacity()
                                    + client->getDemand()) * (1 - arcs[i][j]));
            }
        }
}


void cvrp::Problem::addVariables() {
    vrp::Problem::addVariables();
    consumption = std::vector<IloNumVar>();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        auto *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[0]);
        if (!stock) continue;
        if (i == 0) {
            consumption.push_back(IloNumVar(env,
                                     0, 0,
                                     std::string("q_" + Utilities::itos(i)).c_str()));
        } else {
            if (auto *client = dynamic_cast<routing::attributes::Consumer *>(clients[
                    i - 1]))
                consumption.push_back(IloNumVar( env,
                                         std::max(client->getDemand(), (routing::Demand) 0.0),
                                         std::min(stock->getCapacity(), stock->getCapacity() + client->getDemand()),
                                         std::string("q_" + Utilities::itos(i)).c_str()));
        }
        model.add(consumption.back());
    }
}
