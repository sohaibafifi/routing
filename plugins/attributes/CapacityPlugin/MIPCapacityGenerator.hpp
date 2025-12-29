// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IMIPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/RoutingPlugin/MIPRoutingGenerator.hpp"

#include <vector>
#include <algorithm>

namespace routing {
namespace mip {
namespace generators {

/**
 * @brief MIP Capacity constraint generator
 *
 * Implements vehicle capacity constraints using continuous load variables:
 * - load[i] = cumulative load when arriving at node i
 * - Capacity bounds: load[i] <= capacity
 * - Load propagation: if x[i][j] = 1, then load[j] >= load[i] + demand[j]
 * - Uses big-M formulation for conditional constraints
 */
class MIPCapacityGenerator : public IMIPConstraintGenerator {
public:
    std::string name() const override {
        return "MIPCapacityGenerator";
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

    void setRoutingGenerator(MIPRoutingGenerator* routing) {
        routingGen_ = routing;
    }

    void addVariables(IMIPBackend& mip, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();

        size_t n = clients.size();

        // Get maximum capacity for variable bounds
        double maxCapacity = 0.0;
        for (auto* vehicle : vehicles) {
            auto* stock = vehicle->tryGetAttribute<attributes::Stock>();
            if (stock) {
                maxCapacity = std::max(maxCapacity, static_cast<double>(stock->getCapacity()));
            }
        }

        // Create load variables for each client
        // load[i] = cumulative load when arriving at client i
        loadVars_.clear();
        loadVars_.resize(n + 1);  // Include depot (index 0)

        // Depot load is always 0
        loadVars_[0] = mip.newVar(0.0, 0.0, "load_0");

        // Client load variables
        for (size_t i = 0; i < n; ++i) {
            auto* consumer = clients[i]->tryGetAttribute<attributes::Consumer>();
            double demand = consumer ? static_cast<double>(consumer->getDemand()) : 0.0;

            // Load must be at least the demand of this client
            loadVars_[i + 1] = mip.newVar(demand, maxCapacity, "load_" + std::to_string(i + 1));
        }

        // Store demands and capacities
        demands_.clear();
        for (size_t i = 0; i < n; ++i) {
            auto* consumer = clients[i]->tryGetAttribute<attributes::Consumer>();
            demands_.push_back(consumer ? static_cast<double>(consumer->getDemand()) : 0.0);
        }

        capacities_.clear();
        for (auto* vehicle : vehicles) {
            auto* stock = vehicle->tryGetAttribute<attributes::Stock>();
            capacities_.push_back(stock ? static_cast<double>(stock->getCapacity()) : maxCapacity);
        }

        maxCapacity_ = maxCapacity;
    }

    void addConstraints(IMIPBackend& mip, Problem& problem) override {
        if (!routingGen_) return;  // Need routing generator for arc variables

        size_t n = routingGen_->getNumClients();
        const auto& arcVars = routingGen_->getArcVars();

        if (n == 0) return;

        // Big-M value
        double M = maxCapacity_ * 2.0;

        // Load propagation constraints with big-M
        // If x[i][j] = 1, then load[j] >= load[i] - load_depot[i] + demand[j]
        // For depot: load[0] = 0, so load[j] >= demand[j] when coming from depot
        // For clients: load[j] >= load[i] + demand[j] when coming from client i

        for (size_t i = 0; i <= n; ++i) {
            for (size_t j = 1; j <= n; ++j) {  // j starts from 1 (clients only)
                if (i == j) continue;

                // Load propagation: if x[i][j] = 1, then load[j] >= load[i] + demand[j-1]
                // Reformulation: load[j] - load[i] >= demand[j-1] - M * (1 - x[i][j])
                // Equivalent to: load[i] - load[j] + M * x[i][j] <= M - demand[j-1]

                LinearExpr loadProp;
                loadProp.addTerm(loadVars_[i], 1.0);
                loadProp.addTerm(loadVars_[j], -1.0);
                loadProp.addTerm(arcVars[i][j], M);

                mip.addLessEqual(loadProp, M - demands_[j - 1],
                    "load_prop_" + std::to_string(i) + "_" + std::to_string(j));
            }
        }

        // Capacity constraints are already enforced by variable bounds
        // load[i] <= maxCapacity is in the variable definition
    }

    void addObjectiveTerms(IMIPBackend& mip, Problem& problem, LinearExpr& expr) override {
        // Capacity doesn't typically add to objective
    }

    void extractSolution(const IMIPBackend& mip, Problem& problem, Solution& solution) override {
        // Extract load values
        loads_.clear();
        for (size_t i = 0; i < loadVars_.size(); ++i) {
            loads_.push_back(mip.getValue(loadVars_[i]));
        }
    }

    // Accessors
    const std::vector<Var>& getLoadVars() const { return loadVars_; }
    const std::vector<double>& getDemands() const { return demands_; }
    const std::vector<double>& getCapacities() const { return capacities_; }
    const std::vector<double>& getLoads() const { return loads_; }

private:
    MIPRoutingGenerator* routingGen_ = nullptr;
    std::vector<Var> loadVars_;
    std::vector<double> demands_;
    std::vector<double> capacities_;
    std::vector<double> loads_;  // Extracted solution
    double maxCapacity_ = 0.0;
};

} // namespace generators
} // namespace mip
} // namespace routing
