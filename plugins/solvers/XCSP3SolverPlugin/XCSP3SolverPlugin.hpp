#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "XCSP3Solver.hpp"

namespace routing {
namespace plugins {

class XCSP3SolverPlugin : public IPlugin {
public:
    std::string name() const override { return "XCSP3SolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        // Register XCSP3 solver with type/backend format
        registry.registerSolver("cp/xcsp3",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::XCSP3Solver>(problem);
            });

        // Backward compatibility alias
        registry.registerSolver("xcsp3",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<cp::XCSP3Solver>(problem);
            });
    }
};

} // namespace plugins
} // namespace routing
