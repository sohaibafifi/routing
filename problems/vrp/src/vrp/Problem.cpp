// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Problem.hpp"
#include "models/Tour.hpp"

#include <core/routines/callbacks.hpp>
#include <core/routines/neighborhoods/IDCH.hpp>
#include <core/routines/neighborhoods/Move.hpp>
#include <core/routines/neighborhoods/TwoOpt.hpp>
#include "routines/operators/Constructor.hpp"
#include "routines/operators/Destructor.hpp"
#include "models/Solution.hpp"

routing::callback::HeuristicCallback *vrp::Problem::setHeuristicCallback() {
    std::vector<routing::Neighborhood *> dummyNeighborhoods;
    dummyNeighborhoods.push_back(new routing::IDCH(new vrp::routines::Constructor(), new vrp::routines::Destructor()));
    dummyNeighborhoods.push_back(new routing::TwoOpt());
    dummyNeighborhoods.push_back(new routing::Move(new vrp::routines::Constructor()));

    return new routing::callback::HeuristicCallback(env,
                                                    this,
                                                    new routing::Generator(this, new vrp::routines::Constructor(),
                                                                           new vrp::routines::Destructor()),
                                                    new routing::dummyDiver(),
                                                    dummyNeighborhoods);
}

routing::callback::IncumbentCallback *vrp::Problem::setIncumbentCallback() {
    return new routing::callback::IncumbentCallback(env, this);
}


void vrp::Problem::addVariables() {
    for (unsigned i = 0; i <= clients.size(); ++i) {
        arcs.push_back(std::vector<IloNumVar>());
        for (unsigned j = 0; j <= clients.size(); ++j) {
            std::string name = std::string("X_" + Utilities::itos(i) + "_" + Utilities::itos(j));
            arcs.back().push_back(IloBoolVar(env, name.c_str()));
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
    IloExpr obj_expr(env);
    for (int i = 0; i < clients.size(); ++i) {
        obj_expr += getDistance(*clients[i], *getDepot()) * arcs[i + 1][0];
        obj_expr += getDistance(*clients[i], *getDepot()) * arcs[0][i + 1];
        for (int j = 0; j < clients.size(); ++j) {
            obj_expr += getDistance(*clients[i], *clients[j]) * arcs[i + 1][j + 1];
        }
    }
    obj = IloMinimize(env, obj_expr);
    model.add(obj);
}

void vrp::Problem::addAffectationConstraints() {
    for (unsigned i = 0; i <= clients.size(); ++i) {
        affectation.push_back(std::vector<IloNumVar>());
        if (i == 0) continue;
        for (unsigned k = 0; k < vehicles.size(); ++k) {
            std::string name = std::string("A_" + Utilities::itos(i) + "_" + Utilities::itos(k));
            affectation.back().push_back(IloBoolVar(env, name.c_str()));
            model.add(affectation.back().back());
        }
    }
    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(env);
        for (unsigned k = 0; k < vehicles.size(); ++k) {
            expr += affectation[i][k];
        }
        model.add(IloRange(env, 1,expr , 1, "affectation"));
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
    IloExpr expr(env);
    for (unsigned i = 1; i <= clients.size(); ++i) {
        expr += arcs[0][i];
    }

    model.add(IloRange(env, 0,expr , IloInt(vehicles.size()), "routing-1"));
    expr.end();
    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(env);
        for (unsigned j = 0; j <= clients.size(); ++j) {
            expr += arcs[i][j];
        }
        model.add(IloRange(env, 1,expr , 1, "routing-2"));

        expr.end();
    }


    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(env);
        for (unsigned j = 0; j <= clients.size(); ++j) {
            expr += arcs[i][j] - arcs[j][i];
        }
        model.add(IloRange(env, 0,expr , 0, "routing-3"));
        expr.end();
    }

}

void vrp::Problem::addSequenceConstraints() {

    for (unsigned i = 0; i <= clients.size(); ++i) {
        order.push_back(IloNumVar(env, 0, clients.size(), std::string("o_" + Utilities::itos(i)).c_str()));
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

}



routing::models::Solution *vrp::Initializer::initialSolution() {
    return new models::Solution(this->getProblem());
}

routing::models::Tour *vrp::Initializer::initialTour(int vehicleID) {
    return new models::Tour(this->getProblem(), vehicleID);
}

routing::Duration
vrp::Problem::getDistance(const routing::models::Client &c1, const routing::models::Client &c2) const {
    return distances[c1.getID() - 1][c2.getID() - 1];
}

routing::Duration vrp::Problem::getDistance(const routing::models::Client &c1, const routing::models::Depot &d) const {
    return distances_to_depots[d.getID() - 1][c1.getID() - 1];
}

routing::Memory *vrp::Problem::getMemory() {
    return Memory::get();
}

routing::Memory *routing::Memory::singleton = nullptr;

routing::Memory *vrp::Memory::get() {
    if (!singleton)
        singleton = new vrp::Memory;
    return singleton;
}
