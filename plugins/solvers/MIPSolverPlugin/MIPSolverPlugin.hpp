// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/ISolver.hpp"
#include "plugins/solvers/MIPSolverPlugin/MIPSolver.hpp"

namespace routing {
namespace plugins {

class MIPSolverWrapper : public ISolver {
public:
    explicit MIPSolverWrapper(Problem* problem)
        : solver_(problem) {}

    std::string name() const override { return "mip"; }
    std::string description() const override { return "Exact MIP solver (CPLEX)"; }

    void setProblem(Problem* /*problem*/) override {
        // MIPSolver takes the problem in its constructor.
    }

    Problem* getProblem() const override { return solver_.getProblem(); }

    void setConfiguration(Configuration* config) override {
        solver_.configuration = config;
    }

    void setDefaultConfiguration() override {
        solver_.setDefaultConfiguration();
    }

    bool solve(double timeout) override {
        return solver_.solve(timeout);
    }

    models::Solution* getSolution() const override {
        return solver_.getSolution();
    }

    double getObjectiveValue() const override {
#ifdef CPLEX_FOUND
        try {
            return solver_.getCplex().getObjValue();
        } catch (...) {
            // Fall through to solution cost.
        }
#endif
        auto* solution = solver_.getSolution();
        return solution ? solution->getCost() : 0.0;
    }

    bool isOptimal() const override {
#ifdef CPLEX_FOUND
        return solver_.getCplex().getStatus() == IloAlgorithm::Optimal;
#else
        return false;
#endif
    }

private:
    MIPSolver solver_;
};

class MIPSolverPlugin : public IPlugin {
public:
    std::string name() const override { return "MIPSolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        registry.registerSolver("mip",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolverWrapper>(problem);
            });

        registry.registerSolver("cplex",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolverWrapper>(problem);
            });
    }
};

} // namespace plugins
} // namespace routing
