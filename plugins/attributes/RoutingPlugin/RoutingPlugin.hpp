// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/RoutingPlugin/RoutingConstraintGenerator.hpp"
#include "plugins/attributes/RoutingPlugin/CPRoutingGenerator.hpp"
#include "plugins/attributes/RoutingPlugin/MIPRoutingGenerator.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/RoutingPlugin/RoutingEvaluator.hpp"

namespace routing {
namespace plugins {

class RoutingPlugin : public IPlugin {
public:
    std::string name() const override { return "RoutingPlugin"; }
    std::string description() const override {
        return "Provides base routing constraints";
    }
    PluginType type() const override { return PluginType::ConstraintGenerator; }

    void initialize(PluginRegistry& registry) override {
        // Legacy CPLEX generator
        if (!registry.hasGenerator("RoutingConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::RoutingConstraintGenerator>());
        }

        // CP generator factory
        if (!registry.hasCPGenerator("CPRoutingGenerator")) {
            registry.registerCPGenerator(
                "CPRoutingGenerator",
                []() { return std::make_unique<cp::generators::CPRoutingGenerator>(); },
                { std::type_index(typeid(attributes::GeoNode)) },
                10  // Priority: runs first, provides base variables
            );
        }

        // MIP generator factory
        if (!registry.hasMIPGenerator("MIPRoutingGenerator")) {
            registry.registerMIPGenerator(
                "MIPRoutingGenerator",
                []() { return std::make_unique<mip::generators::MIPRoutingGenerator>(); },
                { std::type_index(typeid(attributes::GeoNode)) },
                10  // Priority: runs first, provides base variables
            );
        }

        if (!registry.hasEvaluator("RoutingEvaluator")) {
            registry.registerEvaluator(
                std::make_unique<evaluators::RoutingEvaluator>());
        }
    }
};

} // namespace plugins
} // namespace routing
