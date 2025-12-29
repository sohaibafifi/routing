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
        // Register CP solvers with type/backend format

        // CP Optimizer (CPLEX backend)
#ifdef CPLEX_FOUND
        registry.registerSolver("cp/cplex",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "cpoptimizer");
            });

#endif

#ifdef ORTOOLS_FOUND
        // OR-Tools CP-SAT backend
        registry.registerSolver("cp/ortools",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "cpsat");
            });
#endif

#if !defined(CPLEX_FOUND) && defined(ORTOOLS_FOUND)
        // Default CP alias when only OR-Tools is available
        registry.registerSolver("cp",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::CPSolver>(problem, "cpsat");
            });
#endif
    }
};

} // namespace plugins
} // namespace routing
