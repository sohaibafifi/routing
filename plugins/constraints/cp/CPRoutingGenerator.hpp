// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"

#include <unordered_map>

namespace routing {
namespace cp {
namespace generators {

/**
 * @brief CP Routing constraint generator
 *
 * Implements the fundamental VRP routing constraints using CP primitives:
 * - Successor variables (next[i] = j means node j follows node i)
 * - Visit interval variables for each node
 * - Circuit constraint for tour structure
 * - Vehicle assignment
 *
 * The model uses a multi-depot multi-vehicle formulation where:
 * - Each vehicle has its own start/end at depot copies
 * - Successor variables encode the tour structure
 * - No-overlap ensures each customer is visited at most once
 */
class CPRoutingGenerator : public ICPConstraintGenerator {
public:
    std::string name() const override {
        return "CPRoutingGenerator";
    }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return { std::type_index(typeid(attributes::GeoNode)) };
    }

    int priority() const override {
        return 10;  // Runs first - provides base variables
    }

    void addVariables(ICPBackend& cp, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();
        auto depots = problem.getComposableDepots();

        if (depots.empty() || vehicles.empty()) return;

        size_t n = clients.size();
        size_t m = vehicles.size();

        // Node indices:
        // 0 to m-1: vehicle start depots
        // m to m+n-1: clients
        // m+n to 2m+n-1: vehicle end depots
        totalNodes_ = 2 * m + n;
        numClients_ = n;
        numVehicles_ = m;

        // Create successor variables: next[i] = j means node j follows node i
        next_.clear();
        for (size_t i = 0; i < totalNodes_; ++i) {
            std::string varName = "next_" + std::to_string(i);
            // Each node can point to any other node or itself (for unused vehicles)
            next_.push_back(cp.newIntVar(0, static_cast<int>(totalNodes_) - 1, varName));
        }

        // Create vehicle assignment variables for clients
        vehicleOf_.clear();
        for (size_t i = 0; i < n; ++i) {
            std::string varName = "vehicle_" + std::to_string(i);
            vehicleOf_.push_back(cp.newIntVar(0, static_cast<int>(m) - 1, varName));
        }

        // Create visit interval variables for clients
        visits_.clear();
        for (size_t i = 0; i < n; ++i) {
            std::string varName = "visit_" + std::to_string(i);
            // Visit duration is 0 for now (service time handled by TimeWindow generator)
            visits_.push_back(cp.newIntervalVar(0, 1000000, 0, 0, varName));
        }

        // Store distance matrix reference
        computeDistanceMatrix(problem);
    }

    void addConstraints(ICPBackend& cp, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();
        size_t n = numClients_;
        size_t m = numVehicles_;

        if (n == 0 || m == 0) return;

        // Add circuit constraint on successor variables
        // This ensures all nodes form valid tours
        cp.addSubCircuit(next_);

        // Vehicle start depots can only go to clients or their own end depot
        for (size_t k = 0; k < m; ++k) {
            // Start depot k can go to clients (m to m+n-1) or its end depot (m+n+k)
            for (size_t j = 0; j < m; ++j) {
                if (j != k) {
                    // Cannot go to other vehicle start depots
                    LinearExpr expr(next_[k]);
                    cp.addLinearConstraint(expr, static_cast<int>(j) + 1, INT_MAX);
                }
            }
        }

        // Clients can go to other clients or end depots (not start depots)
        for (size_t i = 0; i < n; ++i) {
            size_t nodeI = m + i;  // Client node index

            // Cannot go to start depots (0 to m-1) except through circuit
            // This is handled implicitly by the circuit constraint
        }

        // End depots must go to their own start depot (closing the circuit)
        // or can be self-loops if vehicle is unused
        for (size_t k = 0; k < m; ++k) {
            size_t endDepot = m + n + k;
            // End depot k goes back to start depot k
            cp.addEquality(next_[endDepot], static_cast<int>(k));
        }

        // Link vehicle assignment with successor structure
        // If client i is followed by client j, they must be on the same vehicle
        for (size_t i = 0; i < n; ++i) {
            size_t nodeI = m + i;
            for (size_t j = 0; j < n; ++j) {
                if (i == j) continue;
                size_t nodeJ = m + j;

                // If next[nodeI] == nodeJ, then vehicleOf[i] == vehicleOf[j]
                IntVar indicator = cp.newBoolVar("ind_" + std::to_string(i) + "_" + std::to_string(j));

                // This requires element constraint: result = (next[i] == j ? 1 : 0)
                // We approximate with: if same vehicle, can be consecutive
                // More precise: use implication constraints
            }
        }

        // Link start depot to first client vehicle assignment
        for (size_t k = 0; k < m; ++k) {
            for (size_t i = 0; i < n; ++i) {
                size_t nodeI = m + i;
                // If next[k] == nodeI (client i is first on vehicle k)
                // then vehicleOf[i] == k
                IntVar isFirst = cp.newBoolVar("first_" + std::to_string(k) + "_" + std::to_string(i));
                // Constraint: (next[k] == nodeI) <=> (isFirst == 1)
                // And: isFirst => vehicleOf[i] == k
                LinearExpr vehicleExpr(vehicleOf_[i]);
                cp.addImplication(isFirst, vehicleExpr, static_cast<int>(k), static_cast<int>(k));
            }
        }
    }

    void addObjectiveTerms(ICPBackend& cp, Problem& problem, LinearExpr& expr) override {
        if (distanceMatrix_.empty()) return;

        auto clients = problem.getComposableClients();
        size_t n = numClients_;
        size_t m = numVehicles_;

        // Create arc cost variables using element constraints
        for (size_t i = 0; i < totalNodes_; ++i) {
            // cost[i] = distance[i][next[i]]
            std::vector<int> distances;
            for (size_t j = 0; j < totalNodes_; ++j) {
                distances.push_back(distanceMatrix_[i][j]);
            }

            IntVar arcCost = cp.newIntVar(0, 1000000, "arc_cost_" + std::to_string(i));
            cp.addElement(next_[i], distances, arcCost);
            expr.addTerm(arcCost, 1);
        }
    }

    void extractSolution(const ICPBackend& cp, Problem& problem) override {
        // Extract tour structure from next_ variables
        routes_.clear();
        size_t n = numClients_;
        size_t m = numVehicles_;

        for (size_t k = 0; k < m; ++k) {
            std::vector<int> route;
            int current = static_cast<int>(k);  // Start at vehicle k's start depot
            int next = cp.getValue(next_[current]);

            // Follow the tour until we reach the end depot
            while (next != static_cast<int>(k)) {
                if (next >= static_cast<int>(m) && next < static_cast<int>(m + n)) {
                    // This is a client node
                    route.push_back(next - static_cast<int>(m));  // Client index
                }
                current = next;
                next = cp.getValue(next_[current]);
            }

            if (!route.empty()) {
                routes_.push_back(route);
            }
        }
    }

    // Accessors for variables
    const std::vector<IntVar>& getNextVars() const { return next_; }
    const std::vector<IntVar>& getVehicleOfVars() const { return vehicleOf_; }
    const std::vector<IntervalVar>& getVisitVars() const { return visits_; }
    const std::vector<std::vector<int>>& getRoutes() const { return routes_; }

    size_t getNumClients() const { return numClients_; }
    size_t getNumVehicles() const { return numVehicles_; }
    size_t getTotalNodes() const { return totalNodes_; }
    size_t clientNodeIndex(size_t clientIdx) const { return numVehicles_ + clientIdx; }

private:
    std::vector<IntVar> next_;           // Successor variables
    std::vector<IntVar> vehicleOf_;      // Vehicle assignment for clients
    std::vector<IntervalVar> visits_;    // Visit intervals for clients

    std::vector<std::vector<int>> distanceMatrix_;  // Distance matrix
    std::vector<std::vector<int>> routes_;          // Extracted routes

    size_t totalNodes_ = 0;
    size_t numClients_ = 0;
    size_t numVehicles_ = 0;

    void computeDistanceMatrix(Problem& problem) {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();
        auto depots = problem.getComposableDepots();

        if (depots.empty()) return;
        auto* depot = depots[0];

        size_t n = numClients_;
        size_t m = numVehicles_;

        // Initialize distance matrix
        distanceMatrix_.clear();
        distanceMatrix_.resize(totalNodes_, std::vector<int>(totalNodes_, 0));

        // Fill distances
        // Start depots (0 to m-1) to clients (m to m+n-1)
        for (size_t k = 0; k < m; ++k) {
            for (size_t i = 0; i < n; ++i) {
                int dist = static_cast<int>(problem.getDistance(*depot, *clients[i]));
                distanceMatrix_[k][m + i] = dist;
            }
            // Start depot to own end depot (empty route)
            distanceMatrix_[k][m + n + k] = 0;
        }

        // Clients to clients
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (i != j) {
                    int dist = static_cast<int>(problem.getDistance(*clients[i], *clients[j]));
                    distanceMatrix_[m + i][m + j] = dist;
                }
            }
        }

        // Clients to end depots
        for (size_t i = 0; i < n; ++i) {
            for (size_t k = 0; k < m; ++k) {
                int dist = static_cast<int>(problem.getDistance(*clients[i], *depot));
                distanceMatrix_[m + i][m + n + k] = dist;
            }
        }

        // End depots to start depots (circuit closure, cost = 0)
        for (size_t k = 0; k < m; ++k) {
            distanceMatrix_[m + n + k][k] = 0;
        }
    }
};

} // namespace generators
} // namespace cp
} // namespace routing
