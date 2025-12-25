// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/PickupDeliveryPlugin/PickupDeliveryConstraintGenerator.hpp"

namespace routing {
namespace plugins {

class PickupDeliveryPlugin : public IPlugin {
public:
    std::string name() const override { return "PickupDeliveryPlugin"; }
    std::string description() const override {
        return "Provides pickup-delivery capacity constraints";
    }
    PluginType type() const override { return PluginType::Attribute; }

    void initialize(PluginRegistry& registry) override {
        if (!registry.hasGenerator("PickupDeliveryConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::PickupDeliveryConstraintGenerator>());
        }
    }
};

} // namespace plugins
} // namespace routing
