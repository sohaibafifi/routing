// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"

void cvrpstw::Problem::addVariables() {
    cvrp::Problem::addVariables();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        start.push_back(IloNumVar(model.getEnv(),
                                  0,
                                  IloInfinity,
                                  std::string("s_" + Utilities::itos(i)).c_str()));


        wait.push_back(IloNumVar(model.getEnv(),
                                 0,
                                 IloInfinity,
                                 std::string("w_" + Utilities::itos(i)).c_str()));
        delay.push_back(IloNumVar(model.getEnv(),
                                  0,
                                  IloInfinity,
                                  std::string("w_" + Utilities::itos(i)).c_str()));


        model.add(wait.back());
        model.add(delay.back());
        if (i == 0) {
            model.add(wait.back() >= start.back() - static_cast<cvrptw::models::Depot *>(getDepot())->getTwOpen());
            model.add(delay.back() >=  static_cast<cvrptw::models::Depot *>(getDepot())->getTwClose() - start.back());
        }
        else {
            model.add(wait.back() >= start.back() - static_cast<cvrptw::models::Client *>(clients[i - 1])->getTwOpen());
            model.add(wait.back() >= static_cast<cvrptw::models::Client *>(clients[i - 1])->getTwClose() - start.back());
        }


    }

}

void cvrpstw::Problem::addObjective() {
    obj = IloExpr(model.getEnv());
    for (int i = 0; i < clients.size(); ++i) {
        obj += getDistance(*clients[i], *getDepot()) * arcs[i + 1][0];
        obj += getDistance(*clients[i], *getDepot()) * arcs[0][i + 1];
        for (int j = 0; j < clients.size(); ++j) {
            obj += getDistance(*clients[i], *clients[j]) * arcs[i + 1][j + 1];
        }
    }

    for (int i = 0; i < wait.size(); ++i) {
        obj += wait[i] * getWaitPenalty();
        obj += delay[i] * getDelayPenalty();
    }
    model.add(IloMinimize(model.getEnv(), obj));
}

double cvrpstw::Problem::getWaitPenalty() const {
    return waitPenalty;
}

void cvrpstw::Problem::setWaitPenalty(double waitPenalty) {
    Problem::waitPenalty = waitPenalty;
}

double cvrpstw::Problem::getDelayPenalty() const {
    return delayPenalty;
}

void cvrpstw::Problem::setDelayPenalty(double delayPenalty) {
    Problem::delayPenalty = delayPenalty;
}
