// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/ISolver.hpp"
#include "plugins/solvers/PSOSolverPlugin/PSOSolver.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Generator.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"

#include <vector>

namespace routing {
namespace plugins {

class PSOSolverWrapper : public ISolver {
public:
    explicit PSOSolverWrapper(Problem* problem)
        : solver_(problem) {}

    std::string name() const override { return "pso"; }
    std::string description() const override { return "Particle Swarm Optimization solver"; }

    void setProblem(Problem* /*problem*/) override {
        // PSOSolver takes the problem in its constructor.
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

    // Non-ISolver helpers for configuration
    void setGenerator(Generator* generator) {
        solver_.setGenerator(generator);
    }

    void setNeighbors(const std::vector<Neighborhood*>& neighbors) {
        solver_.setNeighbors(neighbors);
    }

    void setBoolParam(const std::string& name, bool value) {
        ensureConfig();
        solver_.configuration->setBoolParam(name, value);
    }

    void setDoubleParam(const std::string& name, double value) {
        ensureConfig();
        solver_.configuration->setDoubleParam(name, value);
    }

    void setIntParam(const std::string& name, int value) {
        ensureConfig();
        solver_.configuration->setIntParam(name, value);
    }

private:
    void ensureConfig() {
        if (!solver_.configuration) {
            solver_.setDefaultConfiguration();
        }
    }

    PSOSolver solver_;
};

class PSOSolverPlugin : public IPlugin {
public:
    std::string name() const override { return "PSOSolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        registry.registerSolver("pso",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<PSOSolverWrapper>(problem);
            });
    }
};

} // namespace plugins
} // namespace routing
