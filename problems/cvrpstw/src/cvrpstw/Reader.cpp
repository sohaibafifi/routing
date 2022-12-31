// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Reader.hpp"
#include "Problem.hpp"

routing::Problem *cvrpstw::Reader::readFile(const std::string & filepath) {
    auto *problem = new cvrpstw::Problem(*dynamic_cast<cvrptw::Problem *>(cvrptw::Reader::readFile(filepath)));
    problem->setDelayPenalty(1);
    problem->setWaitPenalty(1);
    return problem;
}
