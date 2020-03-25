// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Problem.hpp"
#include "models/Tour.hpp"

#ifdef CPLEX
#include <core/routines/callbacks.hpp>
#include "routines/operators/Generator.hpp"
#include "routines/operators/Constructor.hpp"
#include "routines/operators/Destructor.hpp"
#endif
#include "models/Solution.hpp"
#ifdef CPLEX


routing::callback::HeuristicCallback *vrp::Problem::setHeuristicCallback(IloEnv &env) {
    std::vector<routing::Neighborhood *> dummyNeighborhoods;
    return new routing::callback::HeuristicCallback(env,
                                                    this,
                                                    new vrp::routines::Generator(this, new vrp::routines::Constructor(),
                                                                                 new vrp::routines::Destructor()),
                                                    new routing::dummyDiver(),
                                                    dummyNeighborhoods);
}

routing::callback::IncumbentCallback *vrp::Problem::setIncumbentCallback(IloEnv &env) {
    return new routing::callback::IncumbentCallback(env, this);
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

    for (unsigned i = 0; i <= clients.size(); ++i) {
        routing::attributes::Stock *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[0]);
        if (!stock) continue;
        if (i == 0) {
            consumption.push_back(IloNumVar(model.getEnv(),
                                            0, 0,
                                            std::string("q_" + Utilities::itos(i)).c_str()));
        } else {
            if (routing::attributes::Consumer *client = dynamic_cast<routing::attributes::Consumer *>(clients[
                    i - 1]))
                consumption.push_back(IloNumVar(model.getEnv(),
                                                client->getDemand(),
                                                stock->getCapacity(),
                                                std::string("q_" + Utilities::itos(i)).c_str()));
        }
        model.add(consumption.back());
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


void vrp::Problem::addCapacityConstraints() {
    for (auto k = 0; k < vehicles.size(); ++k) {
        IloExpr expr(model.getEnv());
        for (auto i = 1; i <= clients.size(); ++i) {
            if (routing::attributes::Consumer *client = dynamic_cast<routing::attributes::Consumer *>(clients[i - 1]))
                expr += client->getDemand() * affectation[i][k];
        }
        if (routing::attributes::Stock *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[k]))
            model.add(expr <= stock->getCapacity());
    }
    if (routing::attributes::Stock *stock = dynamic_cast<routing::attributes::Stock *>(vehicles[0]))
        for (auto i = 1; i <= clients.size(); ++i) {
            for (auto j = 1; j <= clients.size(); ++j) {
                if (i == j) continue;
                if (routing::attributes::Consumer *client = dynamic_cast<routing::attributes::Consumer *>(clients[j -
                                                                                                                  1]))
                    model.add(consumption[i]
                              + client->getDemand()
                              <= consumption[j]
                                 + (stock->getCapacity()
                                    + client->getDemand()) * (1 - arcs[i][j]));
            }
        }
}

#endif

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
routing::Memory * routing::Memory::singleton = nullptr;
routing::Memory *vrp::Memory::get() {
    if (!singleton)
         singleton = new vrp::Memory;
      return singleton;
}
