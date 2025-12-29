// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "core/interfaces/IMIPConstraintGenerator.hpp"
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

namespace routing {

/**
 * @brief Registry for constraint generators (CP and MIP)
 *
 * Generators register themselves with factories. Solvers query the registry
 * to get all generators whose required attributes are satisfied.
 */
class GeneratorRegistry {
public:
    using CPGeneratorFactory = std::function<std::unique_ptr<cp::ICPConstraintGenerator>()>;
    using MIPGeneratorFactory = std::function<std::unique_ptr<mip::IMIPConstraintGenerator>()>;

    static GeneratorRegistry& instance() {
        static GeneratorRegistry registry;
        return registry;
    }

    // Register a CP generator factory
    void registerCPGenerator(const std::string& name, CPGeneratorFactory factory,
                             std::vector<AttributeTypeId> requiredAttrs, int priority = 100) {
        cpGenerators_[name] = {std::move(factory), std::move(requiredAttrs), priority};
    }

    // Register a MIP generator factory
    void registerMIPGenerator(const std::string& name, MIPGeneratorFactory factory,
                              std::vector<AttributeTypeId> requiredAttrs, int priority = 100) {
        mipGenerators_[name] = {std::move(factory), std::move(requiredAttrs), priority};
    }

    // Get all applicable CP generators for the given enabled attributes
    std::vector<std::unique_ptr<cp::ICPConstraintGenerator>>
    getApplicableCPGenerators(const std::set<AttributeTypeId>& enabledAttrs) const {
        std::vector<std::pair<int, std::unique_ptr<cp::ICPConstraintGenerator>>> generators;

        for (const auto& [name, entry] : cpGenerators_) {
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

    // Get all applicable MIP generators for the given enabled attributes
    std::vector<std::unique_ptr<mip::IMIPConstraintGenerator>>
    getApplicableMIPGenerators(const std::set<AttributeTypeId>& enabledAttrs) const {
        std::vector<std::pair<int, std::unique_ptr<mip::IMIPConstraintGenerator>>> generators;

        for (const auto& [name, entry] : mipGenerators_) {
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

    // List registered generators
    std::vector<std::string> listCPGenerators() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : cpGenerators_) {
            names.push_back(name);
        }
        return names;
    }

    std::vector<std::string> listMIPGenerators() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : mipGenerators_) {
            names.push_back(name);
        }
        return names;
    }

private:
    GeneratorRegistry() = default;

    static bool isApplicable(const std::vector<AttributeTypeId>& required,
                            const std::set<AttributeTypeId>& enabled) {
        for (const auto& attr : required) {
            if (enabled.find(attr) == enabled.end()) {
                return false;
            }
        }
        return true;
    }

    struct CPEntry {
        CPGeneratorFactory factory;
        std::vector<AttributeTypeId> requiredAttrs;
        int priority;
    };

    struct MIPEntry {
        MIPGeneratorFactory factory;
        std::vector<AttributeTypeId> requiredAttrs;
        int priority;
    };

    std::map<std::string, CPEntry> cpGenerators_;
    std::map<std::string, MIPEntry> mipGenerators_;
};

// Registration helper macros
#define ROUTING_REGISTER_CP_GENERATOR(GeneratorClass) \
    namespace { \
        struct GeneratorClass##_CPRegistrar { \
            GeneratorClass##_CPRegistrar() { \
                GeneratorClass temp; \
                routing::GeneratorRegistry::instance().registerCPGenerator( \
                    temp.name(), \
                    []() { return std::make_unique<GeneratorClass>(); }, \
                    temp.requiredAttributes(), \
                    temp.priority() \
                ); \
            } \
        }; \
        static GeneratorClass##_CPRegistrar _##GeneratorClass##_cp_registrar; \
    }

#define ROUTING_REGISTER_MIP_GENERATOR(GeneratorClass) \
    namespace { \
        struct GeneratorClass##_MIPRegistrar { \
            GeneratorClass##_MIPRegistrar() { \
                GeneratorClass temp; \
                routing::GeneratorRegistry::instance().registerMIPGenerator( \
                    temp.name(), \
                    []() { return std::make_unique<GeneratorClass>(); }, \
                    temp.requiredAttributes(), \
                    temp.priority() \
                ); \
            } \
        }; \
        static GeneratorClass##_MIPRegistrar _##GeneratorClass##_mip_registrar; \
    }

} // namespace routing
