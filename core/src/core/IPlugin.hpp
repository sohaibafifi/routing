// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace routing {

class PluginRegistry;

/**
 * @brief Plugin type enumeration
 */
enum class PluginType {
    Attribute,
    ConstraintGenerator,
    Evaluator,
    Solver,
    Neighborhood,
    Reader
};

/**
 * @brief Base interface for all plugins
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;

    /// Get the plugin name (unique identifier)
    virtual std::string name() const = 0;

    /// Get the plugin version
    virtual std::string version() const { return "1.0.0"; }

    /// Get the plugin description
    virtual std::string description() const { return ""; }

    /// Get the plugin type
    virtual PluginType type() const = 0;

    /// Get dependencies (other plugin names this depends on)
    virtual std::vector<std::string> dependencies() const { return {}; }

    /// Initialize the plugin and register components
    virtual void initialize(PluginRegistry& registry) = 0;

    /// Shutdown hook
    virtual void shutdown() {}
};

/**
 * @brief Helper class for static registration
 */
template<typename T>
class PluginRegistrar {
public:
    PluginRegistrar();
};

/**
 * @brief Macro for static plugin registration
 */
#define ROUTING_REGISTER_PLUGIN(PluginClass) \
    static ::routing::PluginRegistrar<PluginClass> \
        _plugin_registrar_##PluginClass{}

} // namespace routing
