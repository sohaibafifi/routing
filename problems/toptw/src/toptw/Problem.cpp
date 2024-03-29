// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.



#include "Problem.hpp"
#include "models/Client.hpp"

#ifdef CPLEX_FOUND

void toptw::Problem::addObjective() {
    IloExpr obj_expr(model.getEnv());
    for (int i = 0; i < clients.size(); ++i) {
        IloExpr presence(model.getEnv());
        for (int j = 0; j < arcs[i + 1].size(); ++j) {
            presence +=  arcs[i + 1][j];
        }
        obj_expr += presence * dynamic_cast<toptw::models::Client*>(clients[i])->getProfit();

    }
    obj = IloMinimize(env, obj_expr);
    model.add(obj);
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
#endif