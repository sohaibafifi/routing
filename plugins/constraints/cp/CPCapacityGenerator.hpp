// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "CPRoutingGenerator.hpp"

namespace routing {
namespace cp {
namespace generators {

/**
 * @brief CP Capacity constraint generator
 *
 * Implements vehicle capacity constraints using CP primitives:
 * - Load variables tracking cumulative demand per vehicle
 * - Capacity constraints ensuring load <= vehicle capacity
 *
 * For each vehicle, the sum of demands of visited clients
 * must not exceed the vehicle's capacity.
 */
class CPCapacityGenerator : public ICPConstraintGenerator {
public:
    std::string name() const override {
        return "CPCapacityGenerator";
    }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return {
            std::type_index(typeid(attributes::Consumer)),
            std::type_index(typeid(attributes::Stock))
        };
    }

    int priority() const override {
        return 50;  // After routing (10), before time windows (60)
    }

    void setRoutingGenerator(CPRoutingGenerator* routing) {
        routingGen_ = routing;
    }

    void addVariables(ICPBackend& cp, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();

        size_t n = clients.size();
        size_t m = vehicles.size();

        if (m == 0) return;

        // Get maximum capacity for variable bounds
        int maxCapacity = 0;
        for (auto* vehicle : vehicles) {
            auto* stock = vehicle->tryGetAttribute<attributes::Stock>();
            if (stock) {
                maxCapacity = std::max(maxCapacity, static_cast<int>(stock->getCapacity()));
            }
        }

        // Load variables for each client: load_i = cumulative load when arriving at i
        loadVars_.clear();
        for (size_t i = 0; i < n; ++i) {
            std::string varName = "load_" + std::to_string(i);
            loadVars_.push_back(cp.newIntVar(0, maxCapacity, varName));
        }

        // Store demands for constraints
        demands_.clear();
        for (size_t i = 0; i < n; ++i) {
            auto* consumer = clients[i]->tryGetAttribute<attributes::Consumer>();
            demands_.push_back(consumer ? static_cast<int>(consumer->getDemand()) : 0);
        }

        // Store capacities
        capacities_.clear();
        for (auto* vehicle : vehicles) {
            auto* stock = vehicle->tryGetAttribute<attributes::Stock>();
            capacities_.push_back(stock ? static_cast<int>(stock->getCapacity()) : maxCapacity);
        }
    }

    void addConstraints(ICPBackend& cp, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();

        size_t n = clients.size();
        size_t m = vehicles.size();

        if (n == 0 || m == 0) return;

        if (routingGen_) {
            // Use vehicle assignment and successor variables from routing generator
            const auto& vehicleOfVars = routingGen_->getVehicleOfVars();
            const auto& nextVars = routingGen_->getNextVars();

            // For each vehicle, constrain total load
            for (size_t k = 0; k < m; ++k) {
                // Sum of demands for clients assigned to vehicle k <= capacity[k]
                LinearExpr loadExpr;

                for (size_t i = 0; i < n; ++i) {
                    // If vehicleOf[i] == k, add demand[i] to the sum
                    IntVar onVehicle = cp.newBoolVar("on_v" + std::to_string(k) + "_" + std::to_string(i));

                    // Link: onVehicle == 1 iff vehicleOf[i] == k
                    cp.addReification(onVehicle, vehicleOfVars[i], static_cast<int>(k));

                    // demand contribution = demand[i] * onVehicle
                    loadExpr.addTerm(onVehicle, demands_[i]);
                }

                // Total load <= capacity
                cp.addLessOrEqual(loadExpr, capacities_[k]);
            }

            // MTZ-style load propagation
            // If next[nodeI] == nodeJ, then load[j] >= load[i] + demand[j]
            for (size_t i = 0; i < n; ++i) {
                size_t nodeI = routingGen_->clientNodeIndex(i);

                for (size_t j = 0; j < n; ++j) {
                    if (i == j) continue;
                    size_t nodeJ = routingGen_->clientNodeIndex(j);

                    // If next[nodeI] == nodeJ, then load[j] >= load[i] + demand[j]
                    IntVar isSucc = cp.newBoolVar("load_succ_" + std::to_string(i) + "_" + std::to_string(j));

                    // Link: isSucc == 1 iff next[nodeI] == nodeJ
                    cp.addReification(isSucc, nextVars[nodeI], static_cast<int>(nodeJ));

                    // Load propagation: if isSucc then load[j] >= load[i] + demand[j]
                    LinearExpr propExpr(loadVars_[j]);
                    propExpr.addTerm(loadVars_[i], -1);

                    cp.addImplication(isSucc, propExpr, demands_[j], INT_MAX);
                }
            }
        } else {
            // Without routing generator, just enforce basic capacity constraint
            // Sum of all demands <= largest capacity
            int maxCap = *std::max_element(capacities_.begin(), capacities_.end());
            int totalDemand = 0;
            for (int d : demands_) {
                totalDemand += d;
            }

            if (totalDemand > maxCap * static_cast<int>(m)) {
                // Infeasible - more demand than total capacity
                // Add infeasible constraint
                IntVar dummy = cp.newBoolVar("infeasible");
                cp.addEquality(LinearExpr(dummy), 2);  // Impossible
            }
        }
    }

    void addObjectiveTerms(ICPBackend& cp, Problem& problem, LinearExpr& expr) override {
        // Capacity doesn't typically add to objective
    }

    void extractSolution(const ICPBackend& cp, Problem& problem) override {
        // Extract load values if needed
        loads_.clear();
        for (size_t i = 0; i < loadVars_.size(); ++i) {
            try {
                loads_.push_back(cp.getValue(loadVars_[i]));
            } catch (...) {
                loads_.push_back(0);
            }
        }
    }

    // Accessors
    const std::vector<IntVar>& getLoadVars() const { return loadVars_; }
    const std::vector<int>& getDemands() const { return demands_; }
    const std::vector<int>& getCapacities() const { return capacities_; }
    const std::vector<int>& getLoads() const { return loads_; }

private:
    CPRoutingGenerator* routingGen_ = nullptr;

    std::vector<IntVar> loadVars_;
    std::vector<int> demands_;
    std::vector<int> capacities_;
    std::vector<int> loads_;  // Extracted solution
};

} // namespace generators
} // namespace cp
} // namespace routing
