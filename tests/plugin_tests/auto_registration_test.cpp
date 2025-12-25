#include <gtest/gtest.h>

#include <core/PluginRegistry.hpp>
#include <plugins/attributes/ComposableCorePlugin/Problem.hpp>
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>
#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>
#include <plugins/PluginBundle.hpp>

#include <algorithm>

TEST(PluginAutoRegistrationTest, pluginsRegisteredAfterCorePluginsCall) {
    // Plugins are registered via explicit registerCorePlugins() call
    routing::plugins::registerCorePlugins();
    auto& registry = routing::PluginRegistry::instance();

    EXPECT_NE(registry.getPlugin("RoutingPlugin"), nullptr);
    EXPECT_NE(registry.getPlugin("CapacityPlugin"), nullptr);
    EXPECT_NE(registry.getPlugin("TimeWindowPlugin"), nullptr);
}

TEST(PluginAutoRegistrationTest, composableProblemUsesRegisteredGenerators) {
    routing::plugins::registerCorePlugins();
    auto& registry = routing::PluginRegistry::instance();
    registry.initializeAll();

    auto generators = registry.availableGenerators();
    EXPECT_NE(std::find(generators.begin(), generators.end(), "RoutingConstraintGenerator"), generators.end());

    routing::ComposableProblem problem;
    problem.enableAttributes<routing::attributes::GeoNode,
                             routing::attributes::Consumer,
                             routing::attributes::Stock>();

    auto active = problem.getActiveGenerators();
    EXPECT_FALSE(active.empty());

    bool hasRouting = false;
    bool hasCapacity = false;
    for (const auto* gen : active) {
        if (gen->name() == "RoutingConstraintGenerator") {
            hasRouting = true;
        }
        if (gen->name() == "CapacityConstraintGenerator") {
            hasCapacity = true;
        }
    }

    EXPECT_TRUE(hasRouting);
    EXPECT_TRUE(hasCapacity);
}
