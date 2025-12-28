// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/ISolver.hpp"
#include "plugins/solvers/GASolverPlugin/GASolver.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Generator.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"

namespace routing {
namespace plugins {

class GASolverWrapper : public ISolver {
public:
    explicit GASolverWrapper(Problem* problem)
        : solver_(problem) {}

    std::string name() const override { return "genetic"; }
    std::string description() const override { return "Genetic Algorithm solver"; }

    void setProblem(Problem* /*problem*/) override {
        // GASolver takes the problem in its constructor.
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

    Solution* getSolution() const override {
        return solver_.getSolution();
    }

    double getObjectiveValue() const override {
        auto* solution = solver_.getSolution();
        return solution ? solution->getCost() : 0.0;
    }

    void setImprovementCallback(ImprovementCallback callback) override {
        solver_.setImprovementCallback(std::move(callback));
    }

    // Non-ISolver helpers for configuration
    void setGenerator(Generator* generator) {
        solver_.setGenerator(generator);
    }

    void setNeighbors(const std::vector<Neighborhood*>& neighbors) {
        solver_.setNeighbors(neighbors);
    }

private:
    GASolver solver_;
};

class GASolverPlugin : public IPlugin {
public:
    std::string name() const override { return "GASolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        registry.registerSolver("genetic",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<GASolverWrapper>(problem);
            });

        registry.registerSolver("ga",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<GASolverWrapper>(problem);
            });
    }
};

} // namespace plugins
} // namespace routing
