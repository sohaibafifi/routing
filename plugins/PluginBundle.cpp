// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include "plugins/PluginBundle.hpp"
#include "core/PluginRegistry.hpp"

namespace routing::plugins {

void registerCorePlugins() {
    auto& registry = PluginRegistry::instance();

    // Attribute plugins
    registry.registerPlugin(std::make_unique<ComposableCorePlugin>());
    registry.registerPlugin(std::make_unique<RoutingPlugin>());
    registry.registerPlugin(std::make_unique<CapacityPlugin>());
    registry.registerPlugin(std::make_unique<TimeWindowPlugin>());
    registry.registerPlugin(std::make_unique<ProfitPlugin>());
    registry.registerPlugin(std::make_unique<PickupDeliveryPlugin>());
    registry.registerPlugin(std::make_unique<SyncPlugin>());

    // Solver plugins
    registry.registerPlugin(std::make_unique<GASolverPlugin>());
    registry.registerPlugin(std::make_unique<LSSolverPlugin>());
    registry.registerPlugin(std::make_unique<MASolverPlugin>());
    registry.registerPlugin(std::make_unique<MIPSolverPlugin>());
    registry.registerPlugin(std::make_unique<PSOSolverPlugin>());
    registry.registerPlugin(std::make_unique<VNSSolverPlugin>());

    // Neighborhood plugins
    registry.registerPlugin(std::make_unique<TwoOptPlugin>());
    registry.registerPlugin(std::make_unique<IDCHPlugin>());

    // Reader plugins
    registry.registerPlugin(std::make_unique<SolomonReaderPlugin>());
    registry.registerPlugin(std::make_unique<TSPLIBReaderPlugin>());
}

} // namespace routing::plugins
