// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "IPlugin.hpp"
#include "core/interfaces/ISolver.hpp"
#include "core/interfaces/INeighborhood.hpp"
#include "core/interfaces/IReader.hpp"
#include "core/interfaces/IConstraintGenerator.hpp"
#include "core/interfaces/IEvaluator.hpp"
#include "core/interfaces/IMIPBackend.hpp"
#include "core/interfaces/ICPBackend.hpp"
#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "core/interfaces/IMIPConstraintGenerator.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace routing {

class Problem;

/**
 * @brief Central registry for all plugins and components
 */
class PluginRegistry {
public:
    // ========== Singleton Access ==========
    static PluginRegistry& instance() {
        static PluginRegistry registry;
        return registry;
    }

    // ========== Plugin Management ==========

    void registerPlugin(std::unique_ptr<IPlugin> plugin) {
        std::string pluginName = plugin->name();
        if (plugins_.count(pluginName)) {
            std::cerr << "[PluginRegistry] Warning: Plugin '" << pluginName
                      << "' already registered, replacing." << std::endl;
        }
        std::cout << "[PluginRegistry] Registered plugin: " << pluginName << std::endl;
        plugins_[pluginName] = std::move(plugin);
    }

    void initializeAll() {
        if (initialized_) return;

        std::vector<std::string> order = resolveDependencies();
        initializedOrder_.clear();
        initializedPlugins_.clear();

        for (const auto& name : order) {
            auto it = plugins_.find(name);
            if (it == plugins_.end()) {
                continue;
            }
            std::cout << "[PluginRegistry] Initializing: " << name << std::endl;
            it->second->initialize(*this);
            initializedPlugins_.insert(name);
            initializedOrder_.push_back(name);
        }

        initialized_ = true;
    }

    void shutdownAll() {
        for (auto it = initializedOrder_.rbegin(); it != initializedOrder_.rend(); ++it) {
            auto pluginIt = plugins_.find(*it);
            if (pluginIt != plugins_.end()) {
                pluginIt->second->shutdown();
            }
        }
        initializedOrder_.clear();
        initializedPlugins_.clear();
        initialized_ = false;
    }

    IPlugin* getPlugin(const std::string& name) const {
        auto it = plugins_.find(name);
        return it != plugins_.end() ? it->second.get() : nullptr;
    }

    // ========== Constraint Generator Registration ==========

    void registerGenerator(std::unique_ptr<IConstraintGenerator> generator) {
        std::string name = generator->name();
        if (generatorByName_.count(name)) {
            std::cerr << "[PluginRegistry] Warning: Generator '" << name
                      << "' already registered, replacing." << std::endl;
        }
        std::cout << "[PluginRegistry] Registered generator: " << name << std::endl;
        generatorByName_[name] = generator.get();
        generators_.push_back(std::move(generator));
    }

    IConstraintGenerator* getGenerator(const std::string& name) const {
        auto it = generatorByName_.find(name);
        return it != generatorByName_.end() ? it->second : nullptr;
    }

    bool hasGenerator(const std::string& name) const {
        return generatorByName_.count(name) > 0;
    }

    std::vector<IConstraintGenerator*> getGenerators(
            const std::set<AttributeTypeId>& enabledAttrs) const {

        std::vector<IConstraintGenerator*> result;
        for (const auto& gen : generators_) {
            if (gen->isApplicable(enabledAttrs)) {
                result.push_back(gen.get());
            }
        }

        std::sort(result.begin(), result.end(),
                  [](const IConstraintGenerator* a, const IConstraintGenerator* b) {
                      return a->priority() < b->priority();
                  });

        return result;
    }

    const std::vector<std::unique_ptr<IConstraintGenerator>>& allGenerators() const {
        return generators_;
    }

    // ========== CP Constraint Generator Registration ==========

    using CPGeneratorFactory = std::function<std::unique_ptr<cp::ICPConstraintGenerator>()>;

    /**
     * @brief Register a CP generator factory
     * @param name Generator name (used for lookup)
     * @param factory Function that creates a new generator instance
     * @param requiredAttrs Attributes required for this generator
     * @param priority Generator priority (lower = earlier execution)
     */
    void registerCPGenerator(const std::string& name, CPGeneratorFactory factory,
                             std::vector<AttributeTypeId> requiredAttrs, int priority = 100) {
        if (cpGeneratorEntries_.count(name)) {
            std::cerr << "[PluginRegistry] Warning: CP Generator '" << name
                      << "' already registered, replacing." << std::endl;
        }
        std::cout << "[PluginRegistry] Registered CP generator: " << name << std::endl;
        cpGeneratorEntries_[name] = {std::move(factory), std::move(requiredAttrs), priority};
    }

    /**
     * @brief Check if a CP generator is registered
     */
    bool hasCPGenerator(const std::string& name) const {
        return cpGeneratorEntries_.count(name) > 0;
    }

    /**
     * @brief Create a single CP generator by name
     */
    std::unique_ptr<cp::ICPConstraintGenerator> createCPGenerator(const std::string& name) const {
        auto it = cpGeneratorEntries_.find(name);
        if (it == cpGeneratorEntries_.end()) {
            return nullptr;
        }
        return it->second.factory();
    }

    /**
     * @brief Create all applicable CP generators for the given enabled attributes
     * @param enabledAttrs Set of enabled attribute type IDs
     * @return Vector of fresh generator instances, sorted by priority
     */
    std::vector<std::unique_ptr<cp::ICPConstraintGenerator>> createCPGenerators(
            const std::set<AttributeTypeId>& enabledAttrs) const {
        std::vector<std::pair<int, std::unique_ptr<cp::ICPConstraintGenerator>>> generators;

        for (const auto& [name, entry] : cpGeneratorEntries_) {
            if (isApplicable(entry.requiredAttrs, enabledAttrs)) {
                auto gen = entry.factory();
                generators.emplace_back(entry.priority, std::move(gen));
            }
        }

        // Sort by priority
        std::sort(generators.begin(), generators.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        std::vector<std::unique_ptr<cp::ICPConstraintGenerator>> result;
        for (auto& [priority, gen] : generators) {
            result.push_back(std::move(gen));
        }
        return result;
    }

    std::vector<std::string> availableCPGenerators() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : cpGeneratorEntries_) {
            names.push_back(name);
        }
        return names;
    }

    // ========== MIP Constraint Generator Registration ==========

    using MIPGeneratorFactory = std::function<std::unique_ptr<mip::IMIPConstraintGenerator>()>;

    /**
     * @brief Register a MIP generator factory
     * @param name Generator name (used for lookup)
     * @param factory Function that creates a new generator instance
     * @param requiredAttrs Attributes required for this generator
     * @param priority Generator priority (lower = earlier execution)
     */
    void registerMIPGenerator(const std::string& name, MIPGeneratorFactory factory,
                              std::vector<AttributeTypeId> requiredAttrs, int priority = 100) {
        if (mipGeneratorEntries_.count(name)) {
            std::cerr << "[PluginRegistry] Warning: MIP Generator '" << name
                      << "' already registered, replacing." << std::endl;
        }
        std::cout << "[PluginRegistry] Registered MIP generator: " << name << std::endl;
        mipGeneratorEntries_[name] = {std::move(factory), std::move(requiredAttrs), priority};
    }

    /**
     * @brief Check if a MIP generator is registered
     */
    bool hasMIPGenerator(const std::string& name) const {
        return mipGeneratorEntries_.count(name) > 0;
    }

    /**
     * @brief Create a single MIP generator by name
     */
    std::unique_ptr<mip::IMIPConstraintGenerator> createMIPGenerator(const std::string& name) const {
        auto it = mipGeneratorEntries_.find(name);
        if (it == mipGeneratorEntries_.end()) {
            return nullptr;
        }
        return it->second.factory();
    }

    /**
     * @brief Create all applicable MIP generators for the given enabled attributes
     * @param enabledAttrs Set of enabled attribute type IDs
     * @return Vector of fresh generator instances, sorted by priority
     */
    std::vector<std::unique_ptr<mip::IMIPConstraintGenerator>> createMIPGenerators(
            const std::set<AttributeTypeId>& enabledAttrs) const {
        std::vector<std::pair<int, std::unique_ptr<mip::IMIPConstraintGenerator>>> generators;

        for (const auto& [name, entry] : mipGeneratorEntries_) {
            if (isApplicable(entry.requiredAttrs, enabledAttrs)) {
                auto gen = entry.factory();
                generators.emplace_back(entry.priority, std::move(gen));
            }
        }

        // Sort by priority
        std::sort(generators.begin(), generators.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        std::vector<std::unique_ptr<mip::IMIPConstraintGenerator>> result;
        for (auto& [priority, gen] : generators) {
            result.push_back(std::move(gen));
        }
        return result;
    }

    std::vector<std::string> availableMIPGenerators() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : mipGeneratorEntries_) {
            names.push_back(name);
        }
        return names;
    }

    // ========== Evaluator Registration ==========

    void registerEvaluator(std::unique_ptr<IEvaluator> evaluator) {
        std::string name = evaluator->name();
        if (evaluatorByName_.count(name)) {
            std::cerr << "[PluginRegistry] Warning: Evaluator '" << name
                      << "' already registered, replacing." << std::endl;
        }
        std::cout << "[PluginRegistry] Registered evaluator: " << name << std::endl;
        evaluatorByName_[name] = evaluator.get();
        evaluators_.push_back(std::move(evaluator));
    }

    IEvaluator* getEvaluator(const std::string& name) const {
        auto it = evaluatorByName_.find(name);
        return it != evaluatorByName_.end() ? it->second : nullptr;
    }

    bool hasEvaluator(const std::string& name) const {
        return evaluatorByName_.count(name) > 0;
    }

    std::vector<IEvaluator*> getEvaluators(
            const std::set<AttributeTypeId>& enabledAttrs) const {
        std::vector<IEvaluator*> result;
        for (const auto& eval : evaluators_) {
            if (eval->isApplicable(enabledAttrs)) {
                result.push_back(eval.get());
            }
        }

        std::sort(result.begin(), result.end(),
                  [](const IEvaluator* a, const IEvaluator* b) {
                      return a->priority() < b->priority();
                  });

        return result;
    }

    const std::vector<std::unique_ptr<IEvaluator>>& allEvaluators() const {
        return evaluators_;
    }

    // ========== Solver Registration ==========

    using SolverFactory = std::function<std::unique_ptr<ISolver>(Problem*)>;

    void registerSolver(const std::string& name, SolverFactory factory) {
        std::cout << "[PluginRegistry] Registered solver: " << name << std::endl;
        solverFactories_[name] = std::move(factory);
    }

    std::unique_ptr<ISolver> createSolver(const std::string& name, Problem* problem) const {
        auto it = solverFactories_.find(name);
        if (it == solverFactories_.end()) {
            throw std::runtime_error("Solver not found: " + name);
        }
        return it->second(problem);
    }

    std::vector<std::string> availableSolvers() const {
        std::vector<std::string> names;
        names.reserve(solverFactories_.size());
        for (const auto& item : solverFactories_) {
            names.push_back(item.first);
        }
        return names;
    }

    // ========== Solver with Backend Registration ==========

    using SolverWithBackendFactory = std::function<std::unique_ptr<ISolver>(Problem*, const std::string&)>;

    void registerSolverWithBackend(const std::string& name, SolverWithBackendFactory factory) {
        std::cout << "[PluginRegistry] Registered solver with backend: " << name << std::endl;
        solverWithBackendFactories_[name] = std::move(factory);
    }

    std::unique_ptr<ISolver> createSolverWithBackend(const std::string& name,
                                                      Problem* problem,
                                                      const std::string& backend) const {
        auto it = solverWithBackendFactories_.find(name);
        if (it == solverWithBackendFactories_.end()) {
            throw std::runtime_error("Solver with backend support not found: " + name);
        }
        return it->second(problem, backend);
    }

    // ========== MIP Backend Registration ==========

    using MIPBackendFactory = std::function<std::unique_ptr<mip::IMIPBackend>()>;

    void registerMIPBackend(const std::string& name, MIPBackendFactory factory) {
        std::cout << "[PluginRegistry] Registered MIP backend: " << name << std::endl;
        mipBackendFactories_[name] = std::move(factory);
    }

    std::unique_ptr<mip::IMIPBackend> createMIPBackend(const std::string& name) const {
        auto it = mipBackendFactories_.find(name);
        if (it == mipBackendFactories_.end()) {
            throw std::runtime_error("MIP backend not found: " + name);
        }
        return it->second();
    }

    std::vector<std::string> availableMIPBackends() const {
        std::vector<std::string> names;
        names.reserve(mipBackendFactories_.size());
        for (const auto& item : mipBackendFactories_) {
            names.push_back(item.first);
        }
        return names;
    }

    // ========== CP Backend Registration ==========

    using CPBackendFactory = std::function<std::unique_ptr<cp::ICPBackend>()>;

    void registerCPBackend(const std::string& name, CPBackendFactory factory) {
        std::cout << "[PluginRegistry] Registered CP backend: " << name << std::endl;
        cpBackendFactories_[name] = std::move(factory);
    }

    std::unique_ptr<cp::ICPBackend> createCPBackend(const std::string& name) const {
        auto it = cpBackendFactories_.find(name);
        if (it == cpBackendFactories_.end()) {
            throw std::runtime_error("CP backend not found: " + name);
        }
        return it->second();
    }

    std::vector<std::string> availableCPBackends() const {
        std::vector<std::string> names;
        names.reserve(cpBackendFactories_.size());
        for (const auto& item : cpBackendFactories_) {
            names.push_back(item.first);
        }
        return names;
    }

    // ========== Neighborhood Registration ==========

    using NeighborhoodFactory = std::function<std::unique_ptr<INeighborhood>()>;

    void registerNeighborhood(const std::string& name, NeighborhoodFactory factory) {
        std::cout << "[PluginRegistry] Registered neighborhood: " << name << std::endl;
        neighborhoodFactories_[name] = std::move(factory);
    }

    std::unique_ptr<INeighborhood> createNeighborhood(const std::string& name) const {
        auto it = neighborhoodFactories_.find(name);
        if (it == neighborhoodFactories_.end()) {
            throw std::runtime_error("Neighborhood not found: " + name);
        }
        return it->second();
    }

    std::vector<std::string> availableNeighborhoods() const {
        std::vector<std::string> names;
        names.reserve(neighborhoodFactories_.size());
        for (const auto& item : neighborhoodFactories_) {
            names.push_back(item.first);
        }
        return names;
    }

    // ========== Reader Registration ==========

    using ReaderFactory = std::function<std::unique_ptr<IReader>()>;

    void registerReader(const std::string& formatName,
                        const std::vector<std::string>& extensions,
                        ReaderFactory factory) {
        std::cout << "[PluginRegistry] Registered reader: " << formatName << std::endl;
        readerFactories_[formatName] = std::move(factory);
        for (const auto& ext : extensions) {
            extensionToFormat_[ext] = formatName;
        }
    }

    std::unique_ptr<IReader> createReader(const std::string& formatName) const {
        auto it = readerFactories_.find(formatName);
        if (it == readerFactories_.end()) {
            throw std::runtime_error("Reader not found: " + formatName);
        }
        return it->second();
    }

    std::unique_ptr<IReader> createReaderForExtension(const std::string& extension) const {
        auto it = extensionToFormat_.find(extension);
        if (it == extensionToFormat_.end()) {
            throw std::runtime_error("No reader for extension: " + extension);
        }
        return createReader(it->second);
    }

    std::vector<std::string> availableReaders() const {
        std::vector<std::string> names;
        names.reserve(readerFactories_.size());
        for (const auto& item : readerFactories_) {
            names.push_back(item.first);
        }
        return names;
    }

    std::vector<std::string> availableGenerators() const {
        std::vector<std::string> names;
        names.reserve(generatorByName_.size());
        for (const auto& item : generatorByName_) {
            names.push_back(item.first);
        }
        return names;
    }

    std::vector<std::string> availableEvaluators() const {
        std::vector<std::string> names;
        names.reserve(evaluatorByName_.size());
        for (const auto& item : evaluatorByName_) {
            names.push_back(item.first);
        }
        return names;
    }

    // ========== Testing/Reset ==========

    void clear() {
        shutdownAll();
        plugins_.clear();
        generators_.clear();
        generatorByName_.clear();
        cpGeneratorEntries_.clear();
        mipGeneratorEntries_.clear();
        evaluators_.clear();
        evaluatorByName_.clear();
        solverFactories_.clear();
        solverWithBackendFactories_.clear();
        mipBackendFactories_.clear();
        cpBackendFactories_.clear();
        neighborhoodFactories_.clear();
        readerFactories_.clear();
        extensionToFormat_.clear();
        initializedOrder_.clear();
        initializedPlugins_.clear();
        initialized_ = false;
    }

private:
    PluginRegistry() = default;

    static bool isApplicable(const std::vector<AttributeTypeId>& required,
                            const std::set<AttributeTypeId>& enabled) {
        for (const auto& attr : required) {
            if (enabled.find(attr) == enabled.end()) {
                return false;
            }
        }
        return true;
    }

    // Generator entry structures for factory-based registration
    struct CPGeneratorEntry {
        CPGeneratorFactory factory;
        std::vector<AttributeTypeId> requiredAttrs;
        int priority;
    };

    struct MIPGeneratorEntry {
        MIPGeneratorFactory factory;
        std::vector<AttributeTypeId> requiredAttrs;
        int priority;
    };

    std::vector<std::string> resolveDependencies() {
        std::vector<std::string> order;
        std::set<std::string> visited;
        std::set<std::string> visiting;

        std::function<void(const std::string&)> visit = [&](const std::string& name) {
            if (visited.count(name)) return;
            if (visiting.count(name)) {
                throw std::runtime_error("Circular dependency: " + name);
            }

            visiting.insert(name);

            auto it = plugins_.find(name);
            if (it == plugins_.end()) {
                std::cerr << "[PluginRegistry] Warning: Dependency '" << name
                          << "' is not registered." << std::endl;
                visiting.erase(name);
                visited.insert(name);
                return;
            }

            for (const auto& dep : it->second->dependencies()) {
                visit(dep);
            }

            visiting.erase(name);
            visited.insert(name);
            order.push_back(name);
        };

        for (const auto& item : plugins_) {
            visit(item.first);
        }

        return order;
    }

    std::map<std::string, std::unique_ptr<IPlugin>> plugins_;
    std::set<std::string> initializedPlugins_;
    std::vector<std::string> initializedOrder_;
    bool initialized_ = false;

    std::vector<std::unique_ptr<IConstraintGenerator>> generators_;
    std::map<std::string, IConstraintGenerator*> generatorByName_;
    std::map<std::string, CPGeneratorEntry> cpGeneratorEntries_;
    std::map<std::string, MIPGeneratorEntry> mipGeneratorEntries_;
    std::vector<std::unique_ptr<IEvaluator>> evaluators_;
    std::map<std::string, IEvaluator*> evaluatorByName_;

    std::map<std::string, SolverFactory> solverFactories_;
    std::map<std::string, SolverWithBackendFactory> solverWithBackendFactories_;
    std::map<std::string, MIPBackendFactory> mipBackendFactories_;
    std::map<std::string, CPBackendFactory> cpBackendFactories_;
    std::map<std::string, NeighborhoodFactory> neighborhoodFactories_;
    std::map<std::string, ReaderFactory> readerFactories_;
    std::map<std::string, std::string> extensionToFormat_;
};

// ========== Static Registration Implementation ==========

template<typename T>
PluginRegistrar<T>::PluginRegistrar() {
    PluginRegistry::instance().registerPlugin(std::make_unique<T>());
}

} // namespace routing
