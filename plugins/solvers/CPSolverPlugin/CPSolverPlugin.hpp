// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "CPSolver.hpp"

namespace routing {
namespace plugins {

/**
 * @brief Plugin that registers the CP solver with the plugin registry
 */
class CPSolverPlugin : public IPlugin {
public:
    std::string name() const override { return "CPSolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        // Register CP solver with multiple aliases
        registry.registerSolver("cp",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "cpoptimizer");
            });

        registry.registerSolver("cpoptimizer",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "cpoptimizer");
            });

        registry.registerSolver("cpo",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "cpoptimizer");
            });

#ifdef ORTOOLS_FOUND
        // Register OR-Tools CP-SAT solver
        registry.registerSolver("cpsat",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "cpsat");
            });

        registry.registerSolver("ortools",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "ortools");
            });
#endif
    }
};

} // namespace plugins
} // namespace routing
