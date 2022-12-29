// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"
#include "models/Client.hpp"


void toptw::Problem::addObjective() {
    obj = IloExpr(model.getEnv());
    for (int i = 0; i < clients.size(); ++i) {
        IloExpr presence(model.getEnv());
        for (int j = 0; j < arcs[i + 1].size(); ++j) {
            presence +=  arcs[i + 1][j];
        }
        obj += presence * dynamic_cast<toptw::models::Client*>(clients[i])->getProfit();

    }
    model.add(IloMaximize(model.getEnv(), obj));
}


void toptw::Problem::addRoutingConstraints() {
    IloExpr expr(model.getEnv());
    for (unsigned i = 1; i <= clients.size(); ++i) {
        expr += arcs[0][i];
    }
    model.add(expr <= IloInt(vehicles.size()));

    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(model.getEnv());
        for (unsigned j = 0; j <= clients.size(); ++j) {
            expr += arcs[i][j] - arcs[j][i];
        }
        model.add(expr == 0);
    }

}

routing::callback::HeuristicCallback *toptw::Problem::setHeuristicCallback() {
    return nullptr;
}

void toptw::Problem::addCapacityConstraints() {

}
