// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Entity.hpp"
#include "Memory.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/IConstraintGenerator.hpp"
#include "core/interfaces/IEvaluator.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"

#include <set>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <string>

#ifdef CPLEX_FOUND
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ilcplex/ilocplexi.h>
#pragma GCC diagnostic pop
#endif

namespace routing {

#ifdef CPLEX_FOUND
    namespace callback {
        class UserCutCallback;
        class HeuristicCallback;
        class IncumbentCallback;
        class LazyConstraintCallback;
        class InformationCallback;
    }
#endif

    // Forward declarations
    class Problem;
    class Solution;
    class Tour;

    /**
     * @brief Initializer interface for creating solutions and tours
     */
    class Initializer {
        Problem* problem_;
    public:
        explicit Initializer(Problem* p_problem) : problem_(p_problem) {}
        virtual ~Initializer() = default;
        Problem* getProblem() const { return problem_; }
        virtual Solution* initialSolution() = 0;
        virtual Tour* initialTour(int vehicleID) = 0;
    };

    /**
     * @brief A composable VRP problem with runtime attribute configuration
     *
     * Problem replaces the inheritance-based problem hierarchy (VRP -> CVRP -> CVRPTW)
     * with a runtime composition model.
     *
     * Usage:
     *   Problem problem;
     *   problem.enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();
     *
     *   auto* client = problem.addClient(1);
     *   client->addAttribute<GeoNode>(10.0, 20.0);
     *   client->addAttribute<Consumer>(15);
     *
     *   problem.generateModel();  // Constraints auto-added based on enabled attrs
     */
    class Problem {
    public:
        Problem() = default;

        virtual ~Problem() {
            // Smart pointers handle cleanup for entities
        }

        // ========== Name Management ==========

        std::string getName() const { return name_; }
        void setName(const std::string& value) { name_ = value; }

        // ========== Attribute Management ==========

        /**
         * @brief Enable an attribute type for this problem
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
         * @brief Add a new client
         */
        Client* addClient(unsigned id) {
            auto entity = std::make_unique<Client>(id);
            Client* ptr = entity.get();
            clients_.push_back(std::move(entity));
            clientPtrs_.push_back(ptr);
            return ptr;
        }

        /**
         * @brief Add a new vehicle
         */
        Vehicle* addVehicle(unsigned id) {
            auto entity = std::make_unique<Vehicle>(id);
            Vehicle* ptr = entity.get();
            vehicles_.push_back(std::move(entity));
            vehiclePtrs_.push_back(ptr);
            return ptr;
        }

        /**
         * @brief Add a new depot
         */
        Depot* addDepot(unsigned id) {
            auto entity = std::make_unique<Depot>(id);
            Depot* ptr = entity.get();
            depots_.push_back(std::move(entity));
            depotPtrs_.push_back(ptr);
            return ptr;
        }

        /**
         * @brief Get all clients
         */
        std::vector<Client*> getClients() const {
            std::vector<Client*> result;
            result.reserve(clients_.size());
            for (const auto& c : clients_) {
                result.push_back(c.get());
            }
            return result;
        }

        /**
         * @brief Get all vehicles
         */
        std::vector<Vehicle*> getVehicles() const {
            std::vector<Vehicle*> result;
            result.reserve(vehicles_.size());
            for (const auto& v : vehicles_) {
                result.push_back(v.get());
            }
            return result;
        }

        /**
         * @brief Get all depots
         */
        std::vector<Depot*> getDepots() const {
            std::vector<Depot*> result;
            result.reserve(depots_.size());
            for (const auto& d : depots_) {
                result.push_back(d.get());
            }
            return result;
        }

        /**
         * @brief Get first depot (convenience)
         */
        Depot* getDepot() const {
            return depots_.empty() ? nullptr : depots_[0].get();
        }

        /**
         * @brief Get number of clients
         */
        size_t numClients() const { return clients_.size(); }

        /**
         * @brief Get number of vehicles
         */
        size_t numVehicles() const { return vehicles_.size(); }

        // Backward compatibility methods (deprecated - use getClients(), getVehicles(), getDepots())
        std::vector<Client*> getComposableClients() const { return getClients(); }
        std::vector<Vehicle*> getComposableVehicles() const { return getVehicles(); }
        std::vector<Depot*> getComposableDepots() const { return getDepots(); }

        /**
         * @brief Get number of depots
         */
        size_t numDepots() const { return depots_.size(); }

        // ========== Legacy compatibility ==========

        // Raw pointer access for legacy solver/callback code
        std::vector<models::Client*> clients;
        std::vector<models::Vehicle*> vehicles;
        std::vector<models::Depot*> depots;

        // Sync raw pointers with smart pointer storage
        void syncLegacyPointers() {
            clients.clear();
            vehicles.clear();
            depots.clear();
            for (const auto& c : clients_) {
                clients.push_back(c.get());
            }
            for (const auto& v : vehicles_) {
                vehicles.push_back(v.get());
            }
            for (const auto& d : depots_) {
                depots.push_back(d.get());
            }
        }

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

        virtual Duration getDistance(const models::Client& c1, const models::Client& c2) const {
            const auto* geo1 = [&]() -> const attributes::GeoNode* {
                if (auto* e1 = dynamic_cast<const Entity*>(&c1)) {
                    return e1->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&c1);
            }();

            const auto* geo2 = [&]() -> const attributes::GeoNode* {
                if (auto* e2 = dynamic_cast<const Entity*>(&c2)) {
                    return e2->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&c2);
            }();

            if (geo1 && geo2) {
                return geo1->distanceTo(*geo2);
            }
            return 0.0;
        }

        virtual Duration getDistance(const models::Client& c1, const models::Depot& d) const {
            const auto* geo1 = [&]() -> const attributes::GeoNode* {
                if (auto* e1 = dynamic_cast<const Entity*>(&c1)) {
                    return e1->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&c1);
            }();

            const auto* geo2 = [&]() -> const attributes::GeoNode* {
                if (auto* e2 = dynamic_cast<const Entity*>(&d)) {
                    return e2->tryGetAttribute<attributes::GeoNode>();
                }
                return dynamic_cast<const attributes::GeoNode*>(&d);
            }();

            if (geo1 && geo2) {
                return geo1->distanceTo(*geo2);
            }
            return 0.0;
        }

        virtual Duration getDistance(const models::Depot& d, const models::Client& c1) const {
            return getDistance(c1, d);
        }

        /**
         * @brief Get distance between two entities
         */
        Duration getDistanceEntity(const Entity& e1, const Entity& e2) const {
            auto* geo1 = e1.tryGetAttribute<attributes::GeoNode>();
            auto* geo2 = e2.tryGetAttribute<attributes::GeoNode>();
            if (geo1 && geo2) {
                return geo1->distanceTo(*geo2);
            }
            return 0.0;
        }

        Duration getDistanceEntity(const Entity* e1, const Entity* e2) const {
            if (!e1 || !e2) return 0.0;
            return getDistanceEntity(*e1, *e2);
        }

        // Backward compatibility methods (deprecated)
        Duration getDistanceComposable(const Entity& e1, const Entity& e2) const {
            return getDistanceEntity(e1, e2);
        }
        Duration getDistanceComposable(const Entity* e1, const Entity* e2) const {
            return getDistanceEntity(e1, e2);
        }

        // ========== Problem Interface ==========

        virtual Memory* getMemory() {
            if (!memory_) {
                memory_ = std::make_unique<Memory>();
            }
            return memory_.get();
        }

        virtual Initializer* initializer();

#ifdef CPLEX_FOUND
        // ========== CPLEX Variables (public for generators) ==========

        IloCplex cplex;
        IloObjective obj;
        IloModel model;
        IloEnv env;

        /// Arc variables: arcs[i][j] = 1 if arc from i to j is used
        std::vector<std::vector<IloNumVar>> arcs;

        /// Vehicle assignment: affectation[i][k] = 1 if client i assigned to vehicle k
        std::vector<std::vector<IloNumVar>> affectation;

        /// Order variables for subtour elimination
        std::vector<IloNumVar> order;

        /// Start time variables (for time window generators)
        std::vector<IloNumVar> startTime;

        virtual IloCplex& generateModel() {
            this->model = IloModel(env);
            this->model.setName(this->getName().c_str());
            this->addVariables();
            this->addConstraints();
            this->addObjective();
            try {
                this->cplex = IloCplex(this->model);
            } catch (IloException& e) {
                std::cout << e.getMessage() << std::endl;
                exit(EXIT_FAILURE);
            }
            return this->cplex;
        }

        virtual callback::HeuristicCallback* setHeuristicCallback() { return nullptr; }
        virtual callback::IncumbentCallback* setIncumbentCallback() { return nullptr; }
        virtual callback::UserCutCallback* setUserCutCallback() { return nullptr; }
        virtual callback::LazyConstraintCallback* setLazyConstraintCallback() { return nullptr; }
        virtual callback::InformationCallback* setInformationCallback() { return nullptr; }

    protected:
        virtual void addVariables() {
            std::cout << "[Problem] Adding variables via "
                      << activeGenerators_.size() << " generators" << std::endl;

            for (auto* gen : activeGenerators_) {
                std::cout << "  - " << gen->name() << std::endl;
                gen->addVariables(*this);
            }
        }

        virtual void addConstraints() {
            std::cout << "[Problem] Adding constraints via "
                      << activeGenerators_.size() << " generators" << std::endl;

            for (auto* gen : activeGenerators_) {
                std::cout << "  - " << gen->name() << std::endl;
                gen->addConstraints(*this);
            }
        }

        virtual void addObjective() {
            std::cout << "[Problem] Building objective" << std::endl;

            IloExpr objExpr(env);

            // Let each generator contribute to the objective
            for (auto* gen : activeGenerators_) {
                gen->addObjectiveTerms(*this, objExpr);
            }

            obj = IloMinimize(env, objExpr);
            model.add(obj);
        }
#endif

    private:
        void refreshActiveComponents() {
            activeGenerators_ = PluginRegistry::instance().getGenerators(enabledAttributes_);
            activeEvaluators_ = PluginRegistry::instance().getEvaluators(enabledAttributes_);
        }

        std::string name_;

        // Enabled attribute types
        std::set<AttributeTypeId> enabledAttributes_;

        // Active components (from registry, based on enabled attributes)
        std::vector<IConstraintGenerator*> activeGenerators_;
        std::vector<IEvaluator*> activeEvaluators_;

        // Entities (owned via smart pointers)
        std::vector<std::unique_ptr<Client>> clients_;
        std::vector<std::unique_ptr<Vehicle>> vehicles_;
        std::vector<std::unique_ptr<Depot>> depots_;

        // Raw pointer cache (for legacy code)
        std::vector<Client*> clientPtrs_;
        std::vector<Vehicle*> vehiclePtrs_;
        std::vector<Depot*> depotPtrs_;

        // Memory singleton
        std::unique_ptr<Memory> memory_;
    };

    // Backward compatibility alias (deprecated)
    using ComposableProblem = Problem;

} // namespace routing

// Include Solution after Problem is defined
#include "Solution.hpp"

namespace routing {

    /**
     * @brief Default initializer for Problem
     */
    class DefaultInitializer : public Initializer {
    public:
        explicit DefaultInitializer(Problem* p_problem) : Initializer(p_problem) {}

        Solution* initialSolution() override {
            return new Solution(getProblem());
        }

        Tour* initialTour(int vehicleID) override {
            return new Tour(getProblem(), vehicleID);
        }
    };

    inline Initializer* Problem::initializer() {
        return new DefaultInitializer(this);
    }

    // Tour implementations that need Problem definition
    inline Tour::Tour(Problem* p_problem, unsigned vehicleID)
        : models::Tour(p_problem, vehicleID), cost_(0) {}

    inline models::Tour* Tour::clone() const {
        auto* copy = new Tour(problem, getID());
        copy->clients_ = clients_;
        copy->cost_ = cost_;
        return copy;
    }

    // Solution implementations that need Problem definition
    inline Solution* Solution::initFromSequence(Problem* problem, std::vector<models::Client*> sequence) {
        auto* sol = new Solution(problem);
        auto* tour = new Tour(problem, 0);
        for (auto* client : sequence) {
            tour->_pushClient(client);
        }
        sol->pushTour(tour);
        return sol;
    }

} // namespace routing
