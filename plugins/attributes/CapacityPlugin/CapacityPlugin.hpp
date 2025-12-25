// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/CapacityPlugin/CapacityConstraintGenerator.hpp"

namespace routing {
namespace plugins {

class CapacityPlugin : public IPlugin {
public:
    std::string name() const override { return "CapacityPlugin"; }
    std::string description() const override {
        return "Provides vehicle capacity constraints";
    }
    PluginType type() const override { return PluginType::Attribute; }

    void initialize(PluginRegistry& registry) override {
        if (!registry.hasGenerator("CapacityConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::CapacityConstraintGenerator>());
        }
    }
};

} // namespace plugins
} // namespace routing
