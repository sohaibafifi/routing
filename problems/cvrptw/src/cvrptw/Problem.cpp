// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"
#include "models/Depot.hpp"

routing::Initializer *cvrptw::Problem::initializer() {
    return new cvrptw::Initializer(this);
}

void cvrptw::Problem::addVariables() {
    cvrp::Problem::addVariables();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        if (i == 0)
            start.push_back(IloNumVar(env,
                                      static_cast<models::Depot *>(getDepot())->getTwOpen(),
                                      static_cast<models::Depot *>(getDepot())->getTwClose(),
                                      std::string("t_" + Utilities::itos(i)).c_str()));
        else
            start.push_back(IloNumVar(env,
                                      static_cast<models::Client *>(clients[i - 1])->getTwOpen(),
                                      static_cast<models::Client *>(clients[i - 1])->getTwClose(),
                                      std::string("t_" + Utilities::itos(i)).c_str()));

        model.add(start.back());
    }

}


void cvrptw::Problem::addSequenceConstraints() {
    for (unsigned i = 1; i <= clients.size(); ++i) {
        model.add(start[i] >= Problem::getDistance(*clients[i - 1], *getDepot()));
        for (unsigned j = 0; j <= clients.size(); ++j) {
            if (i == j) continue;
            if (j == 0)
                model.add(start[i]
                          +
                          (
                                  static_cast<models::Client *>(clients[i - 1])->getService()
                                  + this->getDistance(*clients[i - 1], *getDepot())
                          ) * arcs[i][0]
                          <=
                          start[j]
                          +
                          (
                                  static_cast<models::Client *>(clients[i - 1])->getTwClose()
                                  - static_cast<models::Depot *>(getDepot())->getTwOpen()
                          ) * (1 - arcs[i][0]));
            else
                model.add(start[i]
                          +
                          (
                                  static_cast<models::Client *>(clients[i - 1])->getService()
                                  + this->getDistance(*clients[i - 1], *clients[j - 1])
                          ) * arcs[i][j]
                          <=
                          start[j]
                          +
                          (
                                  static_cast<models::Client *>(clients[i - 1])->getTwClose()
                                  - static_cast<models::Client *>(clients[j - 1])->getTwOpen()
                          ) * (1 - arcs[i][j]));
        }

    }

}

void cvrptw::Problem::addAffectationConstraints() {

}

void cvrptw::Problem::addCapacityConstraints() {
    if (auto *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[0]))
        for (auto i = 1; i <= clients.size(); ++i) {
            for (auto j = 1; j <= clients.size(); ++j) {
                if (i == j) continue;
                if (auto *client = dynamic_cast<routing::attributes::Consumer *>(clients[j -
                                                                                         1]))
                    model.add(consumption[i]
                              + client->getDemand()
                              <= consumption[j]
                                 + (stock->getCapacity()
                                    + client->getDemand()) * (1 - arcs[i][j]));
            }
        }
}
