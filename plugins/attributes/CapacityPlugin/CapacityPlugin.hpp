// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/CapacityPlugin/CapacityConstraintGenerator.hpp"
#include "plugins/attributes/CapacityPlugin/CPCapacityGenerator.hpp"
#include "plugins/attributes/CapacityPlugin/MIPCapacityGenerator.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/CapacityPlugin/CapacityIncrementalEvaluator.hpp"

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
        // Legacy CPLEX generator
        if (!registry.hasGenerator("CapacityConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::CapacityConstraintGenerator>());
        }

        // CP generator factory
        if (!registry.hasCPGenerator("CPCapacityGenerator")) {
            registry.registerCPGenerator(
                "CPCapacityGenerator",
                []() { return std::make_unique<cp::generators::CPCapacityGenerator>(); },
                {
                    std::type_index(typeid(attributes::Consumer)),
                    std::type_index(typeid(attributes::Stock))
                },
                50  // Priority: after routing (10), before time windows (60)
            );
        }

        // MIP generator factory
        if (!registry.hasMIPGenerator("MIPCapacityGenerator")) {
            registry.registerMIPGenerator(
                "MIPCapacityGenerator",
                []() { return std::make_unique<mip::generators::MIPCapacityGenerator>(); },
                {
                    std::type_index(typeid(attributes::Consumer)),
                    std::type_index(typeid(attributes::Stock))
                },
                50  // Priority: after routing (10), before time windows (60)
            );
        }

        if (!registry.hasEvaluator("CapacityIncrementalEvaluator")) {
            registry.registerEvaluator(
                std::make_unique<evaluators::CapacityIncrementalEvaluator>());
        }
    }
};

} // namespace plugins
} // namespace routing
