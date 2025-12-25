// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"

namespace routing::plugins {

class ComposableCorePlugin : public IPlugin {
public:
    std::string name() const override { return "ComposableCorePlugin"; }
    PluginType type() const override { return PluginType::Attribute; }

    void initialize(PluginRegistry& /*registry*/) override {}
};

} // namespace routing::plugins
