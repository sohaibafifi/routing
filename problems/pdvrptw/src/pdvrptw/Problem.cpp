// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"
#include "models/Client.hpp"



routing::callback::HeuristicCallback *pdvrptw::Problem::setHeuristicCallback() {
    return nullptr;
}

void pdvrptw::Problem::addCapacityConstraints() {
    // TODO : Check theses constraints
    cvrptw::Problem::addCapacityConstraints();
}
