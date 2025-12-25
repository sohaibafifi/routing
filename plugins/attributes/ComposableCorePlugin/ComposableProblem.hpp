// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/Problem.hpp"
#include "ComposableEntity.hpp"
#include "ComposableSolution.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/IConstraintGenerator.hpp"
#include "core/interfaces/IEvaluator.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"

#include <set>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

namespace routing {

    // Forward declaration
    class ComposableProblem;

    /**
     * @brief Initializer for ComposableProblem
     */
    class ComposableInitializer : public Initializer {
    public:
        explicit ComposableInitializer(Problem* p_problem)
            : Initializer(p_problem) {}

        models::Solution* initialSolution() override {
            return new ComposableSolution(getProblem());
        }

        models::Tour* initialTour(int vehicleID) override {
            return new ComposableTour(getProblem(), vehicleID);
        }
    };

    /**
     * @brief A composable VRP problem with runtime attribute configuration
     *
     * ComposableProblem replaces the inheritance-based problem hierarchy
     * (VRP → CVRP → CVRPTW) with a runtime composition model.
     *
     * Usage:
     *   ComposableProblem problem;
     *   problem.enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();
     *
     *   auto* client = problem.addClient(1);
     *   client->addAttribute<GeoNode>(10.0, 20.0);
     *   client->addAttribute<Consumer>(15);
     *
     *   problem.generateModel();  // Constraints auto-added based on enabled attrs
     */
    class ComposableProblem : public Problem {
    public:
        ComposableProblem() = default;

        // ========== Attribute Management ==========

        /**
         * @brief Enable an attribute type for this problem
         *
         * Enabling an attribute makes its associated constraint generators
         * and evaluators active for this problem.
         */
        template<typename Attr>
        void enableAttribute() {
            enabledAttributes_.insert(std::type_index(typeid(Attr)));
            refreshActiveComponents();
        }

        /**
         * @brief Enable multiple attribute types at once
         */
        template<typename... Attrs>
        void enableAttributes() {
            (enableAttribute<Attrs>(), ...);
        }

        /**
         * @brief Check if an attribute type is enabled
         */
        template<typename Attr>
        bool hasAttribute() const {
            return enabledAttributes_.count(std::type_index(typeid(Attr))) > 0;
        }

        /**
         * @brief Get all enabled attribute type IDs
         */
        const std::set<AttributeTypeId>& getEnabledAttributes() const {
            return enabledAttributes_;
        }

        // ========== Entity Management ==========

        /**
         * @brief Add a new composable client
         */
        ComposableClient* addClient(unsigned id) {
            auto entity = std::make_unique<ComposableClient>(id);
            ComposableClient* ptr = entity.get();
            composableClients_.push_back(std::move(entity));
            return ptr;
        }

        /**
         * @brief Add a new composable vehicle
         */
        ComposableVehicle* addVehicle(unsigned id) {
            auto entity = std::make_unique<ComposableVehicle>(id);
            ComposableVehicle* ptr = entity.get();
            composableVehicles_.push_back(std::move(entity));
            return ptr;
        }

        /**
         * @brief Add a new composable depot
         */
        ComposableDepot* addDepot(unsigned id) {
            auto entity = std::make_unique<ComposableDepot>(id);
            ComposableDepot* ptr = entity.get();
            composableDepots_.push_back(std::move(entity));
            return ptr;
        }

        /**
         * @brief Get all composable clients
         */
        std::vector<ComposableClient*> getComposableClients() const {
            std::vector<ComposableClient*> result;
            result.reserve(composableClients_.size());
            for (const auto& c : composableClients_) {
                result.push_back(c.get());
            }
            return result;
        }

        /**
         * @brief Get all composable vehicles
         */
        std::vector<ComposableVehicle*> getComposableVehicles() const {
            std::vector<ComposableVehicle*> result;
            result.reserve(composableVehicles_.size());
            for (const auto& v : composableVehicles_) {
                result.push_back(v.get());
            }
            return result;
        }

        /**
         * @brief Get all composable depots
         */
        std::vector<ComposableDepot*> getComposableDepots() const {
            std::vector<ComposableDepot*> result;
            result.reserve(composableDepots_.size());
            for (const auto& d : composableDepots_) {
                result.push_back(d.get());
            }
            return result;
        }

        /**
         * @brief Get number of clients
         */
        size_t numClients() const { return composableClients_.size(); }

        /**
         * @brief Get number of vehicles
         */
        size_t numVehicles() const { return composableVehicles_.size(); }

        // ========== Active Components ==========

        /**
         * @brief Get active constraint generators (sorted by priority)
         */
        const std::vector<IConstraintGenerator*>& getActiveGenerators() const {
            return activeGenerators_;
        }

        /**
         * @brief Get active evaluators (sorted by priority)
         */
        const std::vector<IEvaluator*>& getActiveEvaluators() const {
            return activeEvaluators_;
        }

        // ========== Distance Calculations ==========

        Duration getDistance(const models::Client& c1, const models::Client& c2) const override {
            const auto* geo1 = [&]() -> const attributes::GeoNode* {
                if (auto* e1 = dynamic_cast<const ComposableEntity*>(&c1)) {
                    return e1->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&c1);
            }();

            const auto* geo2 = [&]() -> const attributes::GeoNode* {
                if (auto* e2 = dynamic_cast<const ComposableEntity*>(&c2)) {
                    return e2->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&c2);
            }();

            if (geo1 && geo2) {
                return geo1->distanceTo(*geo2);
            }
            return 0.0;
        }

        Duration getDistance(const models::Client& c1, const models::Depot& d) const override {
            const auto* geo1 = [&]() -> const attributes::GeoNode* {
                if (auto* e1 = dynamic_cast<const ComposableEntity*>(&c1)) {
                    return e1->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&c1);
            }();

            const auto* geo2 = [&]() -> const attributes::GeoNode* {
                if (auto* e2 = dynamic_cast<const ComposableEntity*>(&d)) {
                    return e2->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&d);
            }();

            if (geo1 && geo2) {
                return geo1->distanceTo(*geo2);
            }
            return 0.0;
        }

        /**
         * @brief Get distance between two composable entities by reference
         *
         * Use this when you have ComposableEntity references and need distance.
         */
        Duration getDistanceComposable(const ComposableEntity& e1, const ComposableEntity& e2) const {
            auto* geo1 = e1.tryGetAttribute<attributes::GeoNode>();
            auto* geo2 = e2.tryGetAttribute<attributes::GeoNode>();
            if (geo1 && geo2) {
                return geo1->distanceTo(*geo2);
            }
            return 0.0;
        }

        /**
         * @brief Get distance between two composable entities by pointer
         */
        Duration getDistanceComposable(const ComposableEntity* e1, const ComposableEntity* e2) const {
            if (!e1 || !e2) return 0.0;
            return getDistanceComposable(*e1, *e2);
        }

        // ========== Problem Interface (from base) ==========

        Memory* getMemory() override {
            if (!memory_) {
                memory_ = std::make_unique<Memory>();
            }
            return memory_.get();
        }

        Initializer* initializer() override {
            return new ComposableInitializer(this);
        }

#ifdef CPLEX_FOUND
        // ========== CPLEX Variables (public for generators) ==========

        /// Arc variables: arcs[i][j] = 1 if arc from i to j is used
        std::vector<std::vector<IloNumVar>> arcs;

        /// Vehicle assignment: affectation[i][k] = 1 if client i assigned to vehicle k
        std::vector<std::vector<IloNumVar>> affectation;

        /// Order variables for subtour elimination
        std::vector<IloNumVar> order;

        /// Start time variables (for time window generators)
        std::vector<IloNumVar> startTime;

    protected:
        void addVariables() override {
            std::cout << "[ComposableProblem] Adding variables via "
                      << activeGenerators_.size() << " generators" << std::endl;

            for (auto* gen : activeGenerators_) {
                std::cout << "  - " << gen->name() << std::endl;
                gen->addVariables(*this);
            }
        }

        void addConstraints() override {
            std::cout << "[ComposableProblem] Adding constraints via "
                      << activeGenerators_.size() << " generators" << std::endl;

            for (auto* gen : activeGenerators_) {
                std::cout << "  - " << gen->name() << std::endl;
                gen->addConstraints(*this);
            }
        }

        void addObjective() override {
            std::cout << "[ComposableProblem] Building objective" << std::endl;

            IloExpr objExpr(env);

            // Let each generator contribute to the objective
            for (auto* gen : activeGenerators_) {
                gen->addObjectiveTerms(*this, objExpr);
            }

            // If no generator added anything, default to minimize total distance
            // This will be handled by the RoutingConstraintGenerator

            obj = IloMinimize(env, objExpr);
            model.add(obj);
        }
#endif

    private:
        void refreshActiveComponents() {
            activeGenerators_ = PluginRegistry::instance().getGenerators(enabledAttributes_);
            activeEvaluators_ = PluginRegistry::instance().getEvaluators(enabledAttributes_);
        }

        // Enabled attribute types
        std::set<AttributeTypeId> enabledAttributes_;

        // Active components (from registry, based on enabled attributes)
        std::vector<IConstraintGenerator*> activeGenerators_;
        std::vector<IEvaluator*> activeEvaluators_;

        // Composable entities (owned)
        std::vector<std::unique_ptr<ComposableClient>> composableClients_;
        std::vector<std::unique_ptr<ComposableVehicle>> composableVehicles_;
        std::vector<std::unique_ptr<ComposableDepot>> composableDepots_;

        // Memory singleton
        std::unique_ptr<Memory> memory_;
    };

} // namespace routing
