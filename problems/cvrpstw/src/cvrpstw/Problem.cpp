// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"
#ifdef CPLEX_FOUND

void cvrpstw::Problem::addVariables() {
    cvrp::Problem::addVariables();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        start.push_back(IloNumVar(env,
                                  0,
                                  IloInfinity,
                                  std::string("s_" + std::to_string(i)).c_str()));


        wait.push_back(IloNumVar(env,
                                 0,
                                 IloInfinity,
                                 std::string("w_" + std::to_string(i)).c_str()));
        delay.push_back(IloNumVar(env,
                                  0,
                                  IloInfinity,
                                  std::string("w_" + std::to_string(i)).c_str()));


        model.add(start.back());
        model.add(wait.back());
        model.add(delay.back());
        if (i == 0) {
            model.add(wait.back() >= static_cast<cvrptw::models::Depot *>(getDepot())->getTwOpen() - start.back());
            model.add(delay.back() >= start.back() - static_cast<cvrptw::models::Depot *>(getDepot())->getTwClose());
        }
        else {
            model.add(wait.back() >= static_cast<cvrptw::models::Client *>(clients[i - 1])->getTwOpen() - start.back());
            model.add(wait.back() >= start.back() - static_cast<cvrptw::models::Client *>(clients[i - 1])->getTwClose());
        }


    }

}

void cvrpstw::Problem::addObjective() {
    IloExpr obj_expr (model.getEnv());
    for (int i = 0; i < clients.size(); ++i) {
        obj_expr += getDistance(*clients[i], *getDepot()) * arcs[i + 1][0];
        obj_expr += getDistance(*clients[i], *getDepot()) * arcs[0][i + 1];
        for (int j = 0; j < clients.size(); ++j) {
            obj_expr += getDistance(*clients[i], *clients[j]) * arcs[i + 1][j + 1];
        }
    }
    obj = IloMinimize(env, obj_expr);

    IloExpr obj_penality_expr(model.getEnv());

    for (int i = 0; i < wait.size(); ++i) {
        obj_penality_expr += wait[i] * getWaitPenalty();
        obj_penality_expr += delay[i] * getDelayPenalty();
    }

    obj_penality = IloMinimize(env, obj_penality_expr);


   model.add(IloMinimize(env, IloStaticLex(env,  obj_penality_expr, obj_expr)));
   // model.add(IloMinimize(env,   obj_penality_expr));
}

routing::callback::HeuristicCallback *cvrpstw::Problem::setHeuristicCallback() {
    return nullptr;
}

routing::callback::UserCutCallback *cvrpstw::Problem::setUserCutCallback() {
    return nullptr;
}

routing::callback::LazyConstraintCallback *cvrpstw::Problem::setLazyConstraintCallback() {
    return nullptr;
}

routing::callback::IncumbentCallback *cvrpstw::Problem::setIncumbentCallback() {
    return nullptr;
}
#endif

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
