// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/SyncPlugin/SyncConstraintGenerator.hpp"

namespace routing {
namespace plugins {

class SyncPlugin : public IPlugin {
public:
    std::string name() const override { return "SyncPlugin"; }
    std::string description() const override {
        return "Provides synchronization constraints";
    }
    PluginType type() const override { return PluginType::Attribute; }

    void initialize(PluginRegistry& registry) override {
        if (!registry.hasGenerator("SyncConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::SyncConstraintGenerator>());
        }
    }
};

} // namespace plugins
} // namespace routing
