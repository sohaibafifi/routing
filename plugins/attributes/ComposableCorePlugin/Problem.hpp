// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Entity.hpp"
#include "Memory.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/IConstraintGenerator.hpp"
#include "core/interfaces/IEvaluator.hpp"
#include "core/interfaces/IIncrementalEvaluator.hpp"

// Attribute headers (for auto-enabling)
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"
#include "plugins/attributes/ProfitPlugin/Profiter.hpp"
#include "plugins/attributes/PickupDeliveryPlugin/Pickup.hpp"
#include "plugins/attributes/PickupDeliveryPlugin/Delivery.hpp"
#include "plugins/attributes/TimeWindowPlugin/SoftTimeWindows.hpp"
#include "plugins/attributes/SyncPlugin/Synced.hpp"

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
         * @brief Auto-enable an attribute type if not already enabled
         *
         * Called automatically when Entity::addAttribute() is used.
         * Only enables if the attribute type is not yet enabled.
         */
        template<typename Attr>
        void autoEnableAttribute() {
            if (!hasAttribute<Attr>()) {
                enableAttribute<Attr>();
            }
        }

        /**
         * @brief Auto-enable an attribute by name (for Python bindings)
         */
        void autoEnableAttributeByName(const std::string& name);

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
            ptr->setProblem(this);  // Set parent reference for auto-enabling
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
            ptr->setProblem(this);  // Set parent reference for auto-enabling
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
            ptr->setProblem(this);  // Set parent reference for auto-enabling
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

        // ========== Custom Distance Matrix ==========

        /**
         * @brief Set a custom distance matrix
         * @param data Pointer to row-major distance data
         * @param n Size of the matrix (n x n)
         *
         * The matrix should include all nodes: depot(s) first, then clients.
         * Index 0 is typically the depot, indices 1..n-1 are clients.
         */
        void setDistanceMatrix(const double* data, size_t n) {
            customDistanceMatrix_.resize(n, std::vector<double>(n));
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = 0; j < n; ++j) {
                    customDistanceMatrix_[i][j] = data[i * n + j];
                }
            }
            useCustomDistances_ = true;
        }

        /**
         * @brief Check if custom distances are enabled
         */
        bool hasCustomDistances() const { return useCustomDistances_; }

        /**
         * @brief Clear custom distance matrix
         */
        void clearDistanceMatrix() {
            customDistanceMatrix_.clear();
            useCustomDistances_ = false;
        }

        /**
         * @brief Get distance from custom matrix by node indices
         */
        Duration getCustomDistance(size_t from, size_t to) const {
            if (useCustomDistances_ &&
                from < customDistanceMatrix_.size() &&
                to < customDistanceMatrix_.size()) {
                return customDistanceMatrix_[from][to];
            }
            return 0.0;
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

        IloEnv env;
        IloModel model;
        IloObjective obj;
        IloCplex cplex;

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

        // Cached initializer to avoid repeated allocations.
        std::unique_ptr<Initializer> initializer_;

        // Custom distance matrix (optional, for non-Euclidean distances)
        std::vector<std::vector<double>> customDistanceMatrix_;
        bool useCustomDistances_ = false;
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
        if (!initializer_) {
            initializer_ = std::make_unique<DefaultInitializer>(this);
        }
        return initializer_.get();
    }

    // Entity::addAttribute implementation (needs Problem to be complete)
    template<typename Attr, typename... Args>
    Attr& Entity::addAttribute(Args&&... args) {
        static_assert(std::is_base_of<IAttribute, Attr>::value,
            "Attr must inherit from IAttribute");

        auto attr = std::make_unique<Attr>(std::forward<Args>(args)...);
        Attr* ptr = attr.get();
        attributes_[std::type_index(typeid(Attr))] = std::move(attr);

        // Auto-enable this attribute type on the parent problem
        if (problem_) {
            problem_->autoEnableAttribute<Attr>();
        }

        return *ptr;
    }

    // Problem::autoEnableAttributeByName implementation (for Python bindings)
    inline void Problem::autoEnableAttributeByName(const std::string& name) {
        if (name == "GeoNode" && !hasAttribute<attributes::GeoNode>()) {
            enableAttribute<attributes::GeoNode>();
        } else if (name == "Consumer" && !hasAttribute<attributes::Consumer>()) {
            enableAttribute<attributes::Consumer>();
        } else if (name == "Stock" && !hasAttribute<attributes::Stock>()) {
            enableAttribute<attributes::Stock>();
        } else if (name == "Rendezvous" && !hasAttribute<attributes::Rendezvous>()) {
            enableAttribute<attributes::Rendezvous>();
        } else if (name == "ServiceQuery" && !hasAttribute<attributes::ServiceQuery>()) {
            enableAttribute<attributes::ServiceQuery>();
        } else if (name == "Profiter" && !hasAttribute<attributes::Profiter>()) {
            enableAttribute<attributes::Profiter>();
        } else if (name == "Pickup" && !hasAttribute<attributes::Pickup>()) {
            enableAttribute<attributes::Pickup>();
        } else if (name == "Delivery" && !hasAttribute<attributes::Delivery>()) {
            enableAttribute<attributes::Delivery>();
        } else if (name == "SoftTimeWindows" && !hasAttribute<attributes::SoftTimeWindows>()) {
            enableAttribute<attributes::SoftTimeWindows>();
        } else if (name == "Synced" && !hasAttribute<attributes::Synced>()) {
            enableAttribute<attributes::Synced>();
        }
        // Unknown attribute names are silently ignored (already enabled or not recognized)
    }

    // Tour implementations that need Problem definition
    inline Tour::Tour(Problem* p_problem, unsigned vehicleID)
        : models::Tour(p_problem, vehicleID), cost_(0) {}

    inline void Tour::update() {
        cost_ = 0;
        if (clients_.empty()) {
            return;
        }

        auto* depot = problem ? problem->getDepot() : nullptr;
        if (!depot) {
            return;
        }

        cost_ += problem->getDistance(*clients_.front(), *depot);
        for (size_t i = 1; i < clients_.size(); ++i) {
            cost_ += problem->getDistance(*clients_[i - 1], *clients_[i]);
        }
        cost_ += problem->getDistance(*clients_.back(), *depot);
    }

    inline models::Tour* Tour::clone() const {
        auto* copy = new Tour(problem, getID());
        copy->clients_ = clients_;
        copy->cost_ = cost_;
        return copy;
    }

    inline void Tour::ensureCache() const {
        if (cache_.isValid()) {
            return;
        }

        auto* problem = getProblem();
        if (!problem) {
            return;
        }

        const auto& evaluators = problem->getActiveEvaluators();
        cache_.resize(clients_.size());

        // Let each incremental evaluator build its part of the cache
        for (auto* eval : evaluators) {
            if (auto* incEval = dynamic_cast<IIncrementalEvaluator*>(eval)) {
                incEval->buildCache(*this, cache_);
            }
        }

        cache_.setValid();
        cache_.version++;
    }

    inline InsertionCost* Tour::evaluateInsertion(models::Client* client, unsigned long position) {
        auto* problem = getProblem();
        if (!problem) {
            return new InsertionCost(0, true);
        }
        const auto& evaluators = problem->getActiveEvaluators();
        if (evaluators.empty()) {
            return new InsertionCost(0, true);
        }

        auto* clientEntity = dynamic_cast<Entity*>(client);
        if (!clientEntity) {
            return new InsertionCost(0, true);
        }

        auto* depotEntity = dynamic_cast<Entity*>(problem->getDepot());
        if (!depotEntity) {
            return new InsertionCost(0, true);
        }

        size_t safePos = position;
        if (safePos > clients_.size()) {
            safePos = clients_.size();
        }
        Entity* pred = depotEntity;
        Entity* succ = depotEntity;
        if (safePos > 0) {
            pred = dynamic_cast<Entity*>(clients_[safePos - 1]);
        }
        if (safePos < clients_.size()) {
            succ = dynamic_cast<Entity*>(clients_[safePos]);
        }

        if (!pred || !succ) {
            return new InsertionCost(0, true);
        }

        InsertionContext ctx{clientEntity, static_cast<int>(safePos), pred, succ};
        bool possible = true;
        double delta = 0.0;

        // Try to use incremental evaluation if cache is valid
        bool useIncremental = cache_.isValid();

        for (auto* eval : evaluators) {
            if (useIncremental) {
                if (auto* incEval = dynamic_cast<IIncrementalEvaluator*>(eval)) {
                    // Use O(1) incremental evaluation
                    MoveDelta md = incEval->evaluateInsertionIncremental(*this, cache_, ctx);
                    if (!md.feasible) {
                        possible = false;
                    }
                    delta += md.costDelta;
                    continue;
                }
            }
            // Fallback to O(n) evaluation
            if (!eval->checkFeasibility(*this, ctx)) {
                possible = false;
            }
            delta += eval->evaluateInsertionDelta(*this, ctx);
        }

        return new InsertionCost(delta, possible);
    }

    // Solution implementations that need Problem definition
    inline Solution* Solution::initFromSequence(Problem* problem, std::vector<models::Client*> sequence) {
        for (auto* tour : tours_) {
            delete tour;
        }
        tours_.clear();
        notserved.clear();
        totalCost_ = 0;
        setPenalty(0.0);
        problem_ = problem;

        auto* tour = new Tour(problem, 0);
        for (auto* client : sequence) {
            tour->_pushClient(client);
        }
        pushTour(tour);
        update();
        return this;
    }

} // namespace routing
