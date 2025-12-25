// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/INeighborhood.hpp"
#include "plugins/neighborhoods/TwoOptPlugin/TwoOpt.hpp"

#include <optional>

namespace routing {
namespace plugins {

class TwoOptNeighborhood : public INeighborhood {
public:
    std::string name() const override { return "two_opt"; }
    std::string description() const override { return "2-opt edge exchange"; }

    std::optional<NeighborhoodMove> explore(Solution& /*solution*/) override {
        return std::nullopt;
    }

    void apply(Solution& /*solution*/, const NeighborhoodMove& /*move*/) override {
    }

    bool improve(Solution& solution) override {
        return neighborhood_.look(&solution);
    }

private:
    TwoOpt neighborhood_;
};

class TwoOptPlugin : public IPlugin {
public:
    std::string name() const override { return "TwoOptPlugin"; }
    PluginType type() const override { return PluginType::Neighborhood; }

    void initialize(PluginRegistry& registry) override {
        registry.registerNeighborhood("two_opt", []() -> std::unique_ptr<INeighborhood> {
            return std::make_unique<TwoOptNeighborhood>();
        });
    }
};

} // namespace plugins
} // namespace routing
