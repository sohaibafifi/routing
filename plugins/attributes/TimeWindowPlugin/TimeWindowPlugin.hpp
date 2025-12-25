// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/TimeWindowPlugin/TimeWindowConstraintGenerator.hpp"

namespace routing {
namespace plugins {

class TimeWindowPlugin : public IPlugin {
public:
    std::string name() const override { return "TimeWindowPlugin"; }
    std::string description() const override {
        return "Provides hard time window constraints";
    }
    PluginType type() const override { return PluginType::Attribute; }

    void initialize(PluginRegistry& registry) override {
        if (!registry.hasGenerator("TimeWindowConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::TimeWindowConstraintGenerator>());
        }
    }
};

} // namespace plugins
} // namespace routing
