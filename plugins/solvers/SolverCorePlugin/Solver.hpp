//
// Created by Sohaib LAFIFI on 20/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "core/interfaces/ISolver.hpp"
#include "Configuration.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace routing {

class Solver {

protected:
    Problem *problem{};
    Solution *solution{};
    ImprovementCallback improvementCallback_;

public:
    std::ostream &os;
    Configuration *configuration{};

    explicit Solver(Problem *p_problem, std::ostream &os = std::cout)
        : problem(p_problem), os(os) {}

    virtual ~Solver() = default;

    virtual bool solve(double timeout = 3600) = 0;
    virtual void setDefaultConfiguration() = 0;

    Solution *getSolution() const { return solution; }
    Problem *getProblem() const { return problem; }
    virtual void save(std::ofstream &output) const;

    void setImprovementCallback(ImprovementCallback callback) {
        improvementCallback_ = std::move(callback);
    }

protected:
    void notifyImprovement(Solution* sol, double cost) {
        if (improvementCallback_) {
            improvementCallback_(sol, cost);
        }
    }
};

inline void Solver::save(std::ofstream &output) const {
    output << this->getProblem()->getName()
           << "\t" << getSolution()->getCost()
           << std::endl;
    output.close();
}

} // namespace routing
