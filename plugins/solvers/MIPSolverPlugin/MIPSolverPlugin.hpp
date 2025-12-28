// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/ISolver.hpp"
#include "core/interfaces/IMIPBackend.hpp"
#include "MIPSolver.hpp"
#include "CPLEXMIPBackend.hpp"

namespace routing {
namespace plugins {

/**
 * @brief Plugin for MIP-based solvers with pluggable backends
 *
 * Registers MIP solver variants using different backends:
 * - "mip" / "cplex": CPLEX backend (default)
 * - "gurobi": Gurobi backend (when available)
 * - "highs": HiGHS backend (when available)
 */
class MIPSolverPlugin : public IPlugin {
public:
    std::string name() const override { return "MIPSolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        // Register CPLEX backend factory
        registry.registerMIPBackend("cplex", []() -> std::unique_ptr<mip::IMIPBackend> {
            return std::make_unique<mip::CPLEXMIPBackend>();
        });

        // Register default MIP solver (uses CPLEX)
        registry.registerSolver("mip",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, "cplex");
            });

        // Register CPLEX-specific solver alias
        registry.registerSolver("cplex",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, "cplex");
            });

        // Register factory that accepts backend type parameter
        registry.registerSolverWithBackend("mip",
            [](Problem* problem, const std::string& backend) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, backend);
            });
    }
};

} // namespace plugins
} // namespace routing
