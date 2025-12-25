// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/RoutingPlugin/RoutingConstraintGenerator.hpp"

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
        if (!registry.hasGenerator("RoutingConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::RoutingConstraintGenerator>());
        }
    }
};

} // namespace plugins
} // namespace routing
