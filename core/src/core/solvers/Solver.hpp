//
// Created by Sohaib LAFIFI on 20/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../data/models/Solution.hpp"
#include "../data/Problem.hpp"
#include "../data/Configuration.hpp"
#include <filesystem>
#include <fstream>

namespace routing {

class Solver {

protected:
    routing::Problem *problem{};
    routing::models::Solution *solution{};

public:
    std::ostream &os;
    routing::Configuration *configuration{};

    explicit Solver(routing::Problem *p_problem, std::ostream &os = std::cout)
        : problem(p_problem), os(os) {}

    virtual ~Solver() = default;

    virtual bool solve(double timeout = 3600) = 0;
    virtual void setDefaultConfiguration() = 0;

    routing::models::Solution *getSolution() const { return solution; }
    routing::Problem *getProblem() const { return problem; }
    virtual void save(std::ofstream &output) const;
};

inline void Solver::save(std::ofstream &output) const {
    output << this->getProblem()->getName()
           << "\t" << getSolution()->getCost()
           << std::endl;
    output.close();
}

} // namespace routing
