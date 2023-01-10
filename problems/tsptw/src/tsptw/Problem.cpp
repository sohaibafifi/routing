// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Problem.hpp"

#ifdef CPLEX_FOUND

routing::callback::HeuristicCallback *tsptw::Problem::setHeuristicCallback() {
    return nullptr;
}

routing::callback::UserCutCallback *tsptw::Problem::setUserCutCallback() {
    return nullptr;
}

routing::callback::LazyConstraintCallback *tsptw::Problem::setLazyConstraintCallback() {
    return nullptr;
}

routing::callback::InformationCallback *tsptw::Problem::setInformationCallback() {
    return nullptr;
}

routing::callback::IncumbentCallback *tsptw::Problem::setIncumbentCallback() {
    return nullptr;
}

void tsptw::Problem::addOneVehicleConstraint() {
    IloExpr expr(env);
    for (unsigned i = 1; i <= clients.size(); ++i) {
        expr += arcs[0][i];
    }
    model.add(IloRange(env, 0,expr , 1, "tsp"));
    expr.end();
}

void tsptw::Problem::addCapacityConstraints() {

}

#endif