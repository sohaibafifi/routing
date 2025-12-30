// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/TimeWindowPlugin/TimeWindowConstraintGenerator.hpp"
#include "plugins/attributes/TimeWindowPlugin/CPTimeWindowGenerator.hpp"
#include "plugins/attributes/TimeWindowPlugin/MIPTimeWindowGenerator.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"
#include "plugins/attributes/TimeWindowPlugin/TimeWindowEvaluator.hpp"

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
        // Legacy CPLEX generator
        if (!registry.hasGenerator("TimeWindowConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::TimeWindowConstraintGenerator>());
        }

        // CP generator factory
        if (!registry.hasCPGenerator("CPTimeWindowGenerator")) {
            registry.registerCPGenerator(
                "CPTimeWindowGenerator",
                []() { return std::make_unique<cp::generators::CPTimeWindowGenerator>(); },
                {
                    std::type_index(typeid(attributes::Rendezvous)),
                    std::type_index(typeid(attributes::ServiceQuery))
                },
                60  // Priority: after routing (10) and capacity (50)
            );
        }

        // MIP generator factory
        if (!registry.hasMIPGenerator("MIPTimeWindowGenerator")) {
            registry.registerMIPGenerator(
                "MIPTimeWindowGenerator",
                []() { return std::make_unique<mip::generators::MIPTimeWindowGenerator>(); },
                {
                    std::type_index(typeid(attributes::Rendezvous)),
                    std::type_index(typeid(attributes::ServiceQuery))
                },
                60  // Priority: after routing (10) and capacity (50)
            );
        }

        if (!registry.hasEvaluator("TimeWindowEvaluator")) {
            registry.registerEvaluator(
                std::make_unique<evaluators::TimeWindowEvaluator>());
        }
    }
};

} // namespace plugins
} // namespace routing
