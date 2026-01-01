//
// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/ISolver.hpp"
#include "plugins/solvers/ALNSSolverPlugin/ALNSSolver.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Generator.hpp"
#include "plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp"

namespace routing {
namespace plugins {

class ALNSSolverWrapper : public ISolver {
public:
    explicit ALNSSolverWrapper(Problem* problem)
        : solver_(problem) {}

    std::string name() const override { return "alns"; }
    std::string description() const override { return "Adaptive Large Neighborhood Search solver with multiple destroy/repair operators"; }

    void setProblem(Problem* /*problem*/) override {
        // ALNSSolver takes problem in its constructor.
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

    void setReactionFactor(double factor) {
        solver_.setReactionFactor(factor);
    }

    void setDecayFactor(double factor) {
        solver_.setDecayFactor(factor);
    }

    void setTemperature(double temp) {
        solver_.setTemperature(temp);
    }

    void setCoolingRate(double rate) {
        solver_.setCoolingRate(rate);
    }

    void setSegmentSize(int size) {
        solver_.setSegmentSize(size);
    }

    void setMinTemperature(double temp) {
        solver_.setMinTemperature(temp);
    }

private:
    ALNSSolver solver_;
};

class ALNSSolverPlugin : public IPlugin {
public:
    std::string name() const override { return "ALNSSolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        registry.registerSolver("alns",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<ALNSSolverWrapper>(problem);
            });
    }
};

} // namespace plugins
} // namespace routing
