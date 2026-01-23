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
#include "GurobiMIPBackend.hpp"
#include "HiGHSMIPBackend.hpp"

namespace routing {
namespace plugins {

/**
 * @brief Plugin for MIP-based solvers with pluggable backends
 *
 * Registers MIP solver variants using different backends:
 * - "mip" / "auto": Auto-select best available (CPLEX > Gurobi > HiGHS)
 * - "cplex": CPLEX backend (commercial)
 * - "gurobi": Gurobi backend (commercial)
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

        // Register Gurobi backend factory
#ifdef GUROBI_FOUND
        registry.registerMIPBackend("gurobi", []() -> std::unique_ptr<mip::IMIPBackend> {
            return std::make_unique<mip::GurobiMIPBackend>();
        });
#endif

        // Register HiGHS backend factory
#ifdef HIGHS_FOUND
        registry.registerMIPBackend("highs", []() -> std::unique_ptr<mip::IMIPBackend> {
            return std::make_unique<mip::HiGHSMIPBackend>();
        });
#endif

        // Register MIP solvers with type/backend format

#ifdef CPLEX_FOUND
        registry.registerSolver("mip/cplex",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, "cplex");
            });
#endif

#ifdef GUROBI_FOUND
        registry.registerSolver("mip/gurobi",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MIPSolver>(problem, "gurobi");
            });
#endif

#ifdef HIGHS_FOUND
        registry.registerSolver("mip/highs",
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
