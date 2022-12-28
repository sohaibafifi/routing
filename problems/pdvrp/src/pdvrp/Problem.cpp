// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"
#include "models/Client.hpp"

routing::callback::HeuristicCallback *pdvrp::Problem::setHeuristicCallback(IloEnv &env) {
    return nullptr;
}


void pdvrp::Problem::addCapacityConstraints() {
    auto *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[0]);
    for (auto i = 1; i <= clients.size(); ++i) {
        for (auto j = 1; j <= clients.size(); ++j) {
            if (i == j) continue;
            // FIXME : check this constraint
            auto *client_i = dynamic_cast<pdvrp::models::Client *>(clients[i - 1]);
            auto *client_j = dynamic_cast<pdvrp::models::Client *>(clients[j - 1]);
            model.add(consumption[i]
                      + client_j->getDemand()
                      <= consumption[j]
                         + (stock->getCapacity()) * (1 - arcs[i][j]));

        }
    }
}
