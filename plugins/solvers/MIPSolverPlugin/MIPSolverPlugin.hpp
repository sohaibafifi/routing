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
#include "HiGHSMIPBackend.hpp"

namespace routing {
namespace plugins {

/**
 * @brief Plugin for MIP-based solvers with pluggable backends
 *
 * Registers MIP solver variants using different backends:
 * - "mip" / "auto": Auto-select best available (CPLEX > HiGHS)
 * - "cplex": CPLEX backend (commercial)
 * - "highs": HiGHS backend (open-source)
 */
class MIPSolverPlugin : public IPlugin {
public:
    std::string name() const override { return "MIPSolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        // Register CPLEX backend factory
#ifdef CPLEX_FOUND
        registry.registerMIPBackend("cplex", []() -> std::unique_ptr<mip::IMIPBackend> {
            return std::make_unique<mip::CPLEXMIPBackend>();
        });
#endif

        // Register HiGHS backend factory
#ifdef HIGHS_FOUND
        registry.registerMIPBackend("highs", []() -> std::unique_ptr<mip::IMIPBackend> {
            return std::make_unique<mip::HiGHSMIPBackend>();
        });
#endif

        // Register default MIP solver (auto-selects best backend)
        registry.registerSolver("mip",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, "auto");
            });

#ifdef CPLEX_FOUND
        // Register CPLEX-specific solver alias
        registry.registerSolver("cplex",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, "cplex");
            });
#endif

#ifdef HIGHS_FOUND
        // Register HiGHS-specific solver alias
        registry.registerSolver("highs",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, "highs");
            });
#endif

        // Register factory that accepts backend type parameter
        registry.registerSolverWithBackend("mip",
            [](Problem* problem, const std::string& backend) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, backend);
            });
    }
};

} // namespace plugins
} // namespace routing
