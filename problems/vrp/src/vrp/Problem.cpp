//
// Created by Sohaib LAFIFI on 20/11/2019.
//


#include "Problem.hpp"
#include "models/Depot.hpp"
#include <utilities/Utilities.hpp>
#include <core/routines/callbacks.hpp>


routing::callback::HeuristicCallback *vrp::Problem::setHeuristicCallback(IloEnv &env) {
    return nullptr;
}

routing::callback::IncumbentCallback *vrp::Problem::setIncumbentCallback(IloEnv &env) {
    return new routing::callback::IncumbentCallback(env, this);
}

routing::callback::UserCutCallback *vrp::Problem::setUserCutCallback(IloEnv &env) {
    return nullptr;
}

routing::callback::LazyConstraintCallback *vrp::Problem::setLazyConstraintCallback(IloEnv &env) {
    return nullptr;
}

routing::callback::InformationCallback *vrp::Problem::setInformationCallback(IloEnv &env) {
    return nullptr;
}

routing::Duration
vrp::Problem::getDistance(const routing::models::Client &c1, const routing::models::Client &c2) const {
    return distances[c1.getID() - 1][c2.getID() - 1];
}

routing::Duration vrp::Problem::getDistance(const routing::models::Client &c1, const routing::models::Depot &d) const {
    return distances_to_depots[d.getID() - 1][c1.getID() - 1];
}


void vrp::Problem::addVariables() {
    for (unsigned i = 0; i <= clients.size(); ++i) {
        arcs.push_back(std::vector<IloNumVar>());
        for (unsigned j = 0; j <= clients.size(); ++j) {
            const char *name = std::string("X_" + Utilities::itos(i) + "_" + Utilities::itos(j)).c_str();
            arcs.back().push_back(IloBoolVar(model.getEnv(), name));
            if (i == j) model.add(arcs.back().back() == 0);
            else model.add(arcs.back().back());
        }
    }
}

void vrp::Problem::addConstraints() {
    addAffectationConstraints();
    addRoutingConstraints();
    addSequenceConstraints();
}

void vrp::Problem::addObjective() {
    addTotalDistanceObjective();

}

void vrp::Problem::addAffectationConstraints() {
    for (unsigned i = 0; i <= clients.size(); ++i) {
        affectation.push_back(std::vector<IloNumVar>());
        if (i == 0) continue;
        for (unsigned k = 0; k < vehicles.size(); ++k) {
            const char *name = std::string("A_" + Utilities::itos(i) + "_" + Utilities::itos(k)).c_str();
            affectation.back().push_back(IloBoolVar(model.getEnv(), name));
            model.add(affectation.back().back());
        }
    }
    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(model.getEnv());
        for (unsigned k = 0; k < vehicles.size(); ++k) {
            expr += affectation[i][k];
        }
        model.add(expr == 1);
    }
    for (unsigned i = 1; i <= clients.size(); ++i) {
        for (unsigned j = 1; j <= clients.size(); ++j) {
            if (i == j) continue;
            for (unsigned k = 0; k < vehicles.size(); ++k) {
                model.add(affectation[i][k] - affectation[j][k] <= 1 - arcs[i][j]);
                model.add(affectation[j][k] - affectation[i][k] <= 1 - arcs[i][j]);
                model.add(affectation[j][k] <= 3 - (arcs[0][i] + arcs[0][j] + affectation[i][k]));
            }
        }
    }

}

void vrp::Problem::addRoutingConstraints() {
    IloExpr expr(model.getEnv());
    for (unsigned i = 1; i <= clients.size(); ++i) {
        expr += arcs[0][i];
    }
    model.add(expr <= IloInt(vehicles.size()));
    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(model.getEnv());
        for (unsigned j = 0; j <= clients.size(); ++j) {
            expr += arcs[i][j];
        }
        model.add(expr == 1);
    }
    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(model.getEnv());
        for (unsigned j = 0; j <= clients.size(); ++j) {
            expr += arcs[i][j] - arcs[j][i];
        }
        model.add(expr == 0);
    }

}

void vrp::Problem::addSequenceConstraints() {
    for (unsigned i = 0; i <= clients.size(); ++i) {
        order.push_back(IloNumVar(model.getEnv(), 0, clients.size(), std::string("o_" + Utilities::itos(i)).c_str()));
        model.add(order.back());
    }
    for (unsigned i = 1; i <= clients.size(); ++i) {
        for (unsigned j = 0; j <= clients.size(); ++j) {
            if (i == j) continue;
            model.add(order[i]
                      + 1
                      - order[j]
                      - clients.size() * (1 - arcs[i][j]) <= 0);
        }
    }
    for (unsigned i = 1; i <= clients.size(); ++i) {
        for (unsigned j = 0; j <= clients.size(); ++j) {
            if (i == j) continue;
            model.add(
                    order[i] - order[j]
                    + IloInt(clients.size() - 1) * arcs[i][j]
                    + IloInt(clients.size() - 3) * arcs[j][i]
                    <= IloInt(clients.size() - 2)
            );
        }
    }
}

void vrp::Problem::addTotalDistanceObjective() {
    obj = IloExpr(model.getEnv());
    for (int i = 0; i < clients.size(); ++i) {
        obj += getDistance(*clients[i], *getDepot()) * arcs[i + 1][0];
        obj += getDistance(*clients[i], *getDepot()) * arcs[0][i + 1];
        for (int j = 0; j < clients.size(); ++j) {
            obj += getDistance(*clients[i], *clients[j]) * arcs[i + 1][j + 1];
        }
    }
    model.add(IloMinimize(model.getEnv(), obj));
}