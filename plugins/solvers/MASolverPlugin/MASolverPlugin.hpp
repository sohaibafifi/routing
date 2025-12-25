// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/ISolver.hpp"
#include "plugins/solvers/MASolverPlugin/MASolver.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Generator.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"

#include <vector>

namespace routing {
namespace plugins {

class MASolverWrapper : public ISolver {
public:
    explicit MASolverWrapper(Problem* problem)
        : solver_(problem) {}

    std::string name() const override { return "memetic"; }
    std::string description() const override { return "Memetic Algorithm solver"; }

    void setProblem(Problem* /*problem*/) override {
        // MASolver takes the problem in its constructor.
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

private:
    MASolver solver_;
};

class MASolverPlugin : public IPlugin {
public:
    std::string name() const override { return "MASolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        registry.registerSolver("memetic",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MASolverWrapper>(problem);
            });

        registry.registerSolver("ma",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MASolverWrapper>(problem);
            });
    }
};

} // namespace plugins
} // namespace routing
