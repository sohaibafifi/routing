//
// Created by ali on 3/28/19.
//

#include "Problem.h"
#include "../../../Utility.hpp"

void CVRPTW::Problem::addVariables()
{
    CVRP::Problem::addVariables();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        if (i == 0)
            start.push_back(IloNumVar(model.getEnv(),
                                      static_cast<Depot*>(getDepot())->getTwOpen(),
                                      static_cast<Depot*>(getDepot())->getTwClose(),
                                      std::string("t_" + Utilities::itos(i)).c_str()));
        else
            start.push_back(IloNumVar(model.getEnv(),
                                      static_cast<Client *>(clients[i - 1])->getTwOpen(),
                                      static_cast<Client *>(clients[i - 1])->getTwClose(),
                                      std::string("t_" + Utilities::itos(i)).c_str()));

        model.add(start.back());
    }
}



void CVRPTW::Problem::addSequenceConstraints()
{
    CVRP::Problem::addSequenceConstraints();
    for (unsigned i = 1; i <= clients.size(); ++i) {
        model.add(start[i] >= CVRPTW::Problem::getDistance(*clients[i - 1], *getDepot()));
        for (unsigned j = 0; j <= clients.size(); ++j) {
            if (i == j) continue;
            if (j == 0)
                model.add(start[i]
                          +
                          (
                                  static_cast<Client *>(clients[i - 1])->getService()
                                  + CVRPTW::Problem::getDistance(*clients[i - 1], *getDepot())
                          ) * arcs[i][0]
                          <=
                          start[j]
                          +
                          (
                                  static_cast<Client *>(clients[i - 1])->getTwClose()
                                  - static_cast<Depot *>(getDepot())->getTwOpen()
                          ) * (1 - arcs[i][0]));
            else
                model.add(start[i]
                          +
                          (
                                  static_cast<Client *>(clients[i - 1])->getService()
                                  + routing::Problem::getDistance(*clients[i - 1], *clients[j - 1])
                          ) * arcs[i][j]
                          <=
                          start[j]
                          +
                          (
                                  static_cast<Client *>(clients[i - 1])->getTwClose()
                                  - static_cast<Client *>(clients[j - 1])->getTwOpen()
                          ) * (1 - arcs[i][j]));
        }

    }
}
