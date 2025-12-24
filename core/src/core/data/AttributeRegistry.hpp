// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "attributes.hpp"
#include "IConstraintGenerator.hpp"
#include "IEvaluator.hpp"

#include <vector>
#include <set>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iostream>

namespace routing {

    /**
     * @brief Central registry for attributes, constraint generators, and evaluators
     *
     * The AttributeRegistry is a singleton that manages the mapping between
     * attributes and their associated constraint generators/evaluators.
     *
     * Usage:
     *   // Registration (typically via macros at static init time)
     *   AttributeRegistry::instance().registerGenerator(
     *       std::make_unique<CapacityConstraintGenerator>());
     *
     *   // Query (by ComposableProblem)
     *   auto generators = AttributeRegistry::instance().getGenerators(enabledAttrs);
     */
    class AttributeRegistry {
    public:
        /// Get the singleton instance
        static AttributeRegistry& instance() {
            static AttributeRegistry registry;
            return registry;
        }

        // Prevent copying
        AttributeRegistry(const AttributeRegistry&) = delete;
        AttributeRegistry& operator=(const AttributeRegistry&) = delete;

        /**
         * @brief Register a constraint generator
         *
         * The generator will be activated when all its required attributes
         * are enabled on a problem.
         */
        void registerGenerator(std::unique_ptr<IConstraintGenerator> generator) {
            std::cout << "[AttributeRegistry] Registered generator: " << generator->name() << std::endl;
            generators_.push_back(std::move(generator));
        }

        /**
         * @brief Register an evaluator
         */
        void registerEvaluator(std::unique_ptr<IEvaluator> evaluator) {
            std::cout << "[AttributeRegistry] Registered evaluator: " << evaluator->name() << std::endl;
            evaluators_.push_back(std::move(evaluator));
        }

        /**
         * @brief Get all applicable constraint generators for given attributes
         *
         * Returns generators sorted by priority (lower = earlier).
         */
        std::vector<IConstraintGenerator*> getGenerators(
            const std::set<AttributeTypeId>& enabledAttrs) const {

            std::vector<IConstraintGenerator*> result;
            for (const auto& gen : generators_) {
                if (gen->isApplicable(enabledAttrs)) {
                    result.push_back(gen.get());
                }
            }

            // Sort by priority
            std::sort(result.begin(), result.end(),
                [](const IConstraintGenerator* a, const IConstraintGenerator* b) {
                    return a->priority() < b->priority();
                });

            return result;
        }

        /**
         * @brief Get all applicable evaluators for given attributes
         *
         * Returns evaluators sorted by priority (lower = earlier).
         */
        std::vector<IEvaluator*> getEvaluators(
            const std::set<AttributeTypeId>& enabledAttrs) const {

            std::vector<IEvaluator*> result;
            for (const auto& eval : evaluators_) {
                if (eval->isApplicable(enabledAttrs)) {
                    result.push_back(eval.get());
                }
            }

            // Sort by priority
            std::sort(result.begin(), result.end(),
                [](const IEvaluator* a, const IEvaluator* b) {
                    return a->priority() < b->priority();
                });

            return result;
        }

        /**
         * @brief Get all registered generators (for debugging)
         */
        const std::vector<std::unique_ptr<IConstraintGenerator>>& allGenerators() const {
            return generators_;
        }

        /**
         * @brief Get all registered evaluators (for debugging)
         */
        const std::vector<std::unique_ptr<IEvaluator>>& allEvaluators() const {
            return evaluators_;
        }

        /**
         * @brief Clear all registrations (mainly for testing)
         */
        void clear() {
            generators_.clear();
            evaluators_.clear();
        }

    private:
        AttributeRegistry() = default;

        std::vector<std::unique_ptr<IConstraintGenerator>> generators_;
        std::vector<std::unique_ptr<IEvaluator>> evaluators_;
    };

    /**
     * @brief Helper for auto-registration of constraint generators
     *
     * Usage:
     *   // In .hpp or .cpp file:
     *   ROUTING_REGISTER_GENERATOR(MyConstraintGenerator)
     */
    template<typename T>
    struct GeneratorRegistrar {
        GeneratorRegistrar() {
            AttributeRegistry::instance().registerGenerator(std::make_unique<T>());
        }
    };

    /**
     * @brief Helper for auto-registration of evaluators
     */
    template<typename T>
    struct EvaluatorRegistrar {
        EvaluatorRegistrar() {
            AttributeRegistry::instance().registerEvaluator(std::make_unique<T>());
        }
    };

} // namespace routing

// Macro for auto-registering constraint generators
#define ROUTING_REGISTER_GENERATOR(Class) \
    static ::routing::GeneratorRegistrar<Class> _registrar_##Class{}

// Macro for auto-registering evaluators
#define ROUTING_REGISTER_EVALUATOR(Class) \
    static ::routing::EvaluatorRegistrar<Class> _registrar_##Class{}
