// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/INeighborhood.hpp"
#include "plugins/neighborhoods/IDCHPlugin/IDCH.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Constructor.hpp"
#include "plugins/solvers/OperatorsPlugin/operators/Destructor.hpp"

#include <memory>
#include <optional>

namespace routing {
namespace plugins {

class IDCHNeighborhood : public INeighborhood {
public:
    IDCHNeighborhood() = default;

    explicit IDCHNeighborhood(Constructor* constructor, Destructor* destructor)
        : constructor_(constructor), destructor_(destructor) {
        rebuild();
    }

    std::string name() const override { return "idch"; }
    std::string description() const override { return "Iterated Destruction-Construction"; }

    std::optional<NeighborhoodMove> explore(models::Solution& /*solution*/) override {
        return std::nullopt;
    }

    void apply(models::Solution& /*solution*/, const NeighborhoodMove& /*move*/) override {
    }

    bool improve(models::Solution& solution) override {
        if (!idch_) {
            return false;
        }
        return idch_->look(&solution);
    }

    void setOperators(Constructor* constructor, Destructor* destructor) {
        constructor_ = constructor;
        destructor_ = destructor;
        rebuild();
    }

private:
    void rebuild() {
        if (constructor_ && destructor_) {
            idch_ = std::make_unique<IDCH>(constructor_, destructor_);
        } else {
            idch_.reset();
        }
    }

    Constructor* constructor_ = nullptr;
    Destructor* destructor_ = nullptr;
    std::unique_ptr<IDCH> idch_;
};

class IDCHPlugin : public IPlugin {
public:
    std::string name() const override { return "IDCHPlugin"; }
    PluginType type() const override { return PluginType::Neighborhood; }

    void initialize(PluginRegistry& registry) override {
        registry.registerNeighborhood("idch", []() -> std::unique_ptr<INeighborhood> {
            return std::make_unique<IDCHNeighborhood>();
        });
    }
};

} // namespace plugins
} // namespace routing
