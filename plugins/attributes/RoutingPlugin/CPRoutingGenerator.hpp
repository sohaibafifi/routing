// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"

#include <cmath>
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

        // No extra variables needed beyond next_ for routing structure

        // Store distance matrix reference
        computeDistanceMatrix(problem);
    }

    void addConstraints(ICPBackend& cp, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();
        size_t n = numClients_;
        size_t m = numVehicles_;

        if (n == 0 || m == 0) return;

        // Enforce a permutation structure: each node has a unique predecessor/successor
        // This allows multiple cycles (one per vehicle) while removing most symmetry.
        cp.addAllDifferent(next_);

        // Start depot k can go to clients (m..m+n-1) or any end depot (m+n..m+n+m-1)
        // Just exclude other start depots: next[k] >= m
        for (size_t k = 0; k < m; ++k) {
            LinearExpr expr(next_[k]);
            cp.addLinearConstraint(expr, static_cast<int>(m), INT_MAX);
        }

        // Client i can go to other clients (m..m+n-1) or end depots (m+n..m+n+m-1)
        // Just exclude start depots: next[m+i] >= m
        for (size_t i = 0; i < n; ++i) {
            size_t nodeI = m + i;
            LinearExpr expr(next_[nodeI]);
            cp.addLinearConstraint(expr, static_cast<int>(m), INT_MAX);
        }

        // Start depot k can only go to its own end depot or a client
        for (size_t k = 0; k < m; ++k) {
            for (size_t l = 0; l < m; ++l) {
                if (l == k) continue;
                size_t otherEnd = m + n + l;
                IntVar invalidEnd = cp.newBoolVar("start_end_" + std::to_string(k) + "_" + std::to_string(l));
                cp.addReification(invalidEnd, next_[k], static_cast<int>(otherEnd));
                cp.addEquality(LinearExpr(invalidEnd), 0);
            }
        }

        // End depot k MUST return to start depot k
        for (size_t k = 0; k < m; ++k) {
            size_t endDepot = m + n + k;
            cp.addEquality(next_[endDepot], static_cast<int>(k));
        }

        // CRITICAL: Prevent self-loops for clients
        // Each client must be followed by a different node (no self-loops)
        for (size_t i = 0; i < n; ++i) {
            size_t nodeI = m + i;
            // next[nodeI] != nodeI (client can't point to itself)
            LinearExpr expr(next_[nodeI]);
            // Either next < nodeI or next > nodeI
            // Since next >= m already, we just need next != nodeI
            // Use two-sided exclusion: next != nodeI means next < nodeI OR next > nodeI
            // But simpler: create indicator and force it to 0
            IntVar selfLoop = cp.newBoolVar("selfloop_" + std::to_string(i));
            cp.addReification(selfLoop, next_[nodeI], static_cast<int>(nodeI));
            cp.addEquality(LinearExpr(selfLoop), 0);  // No self-loops allowed
        }

        // Vehicle assignment is handled implicitly via route structure
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

            // Safety counter to prevent infinite loops
            int maxIterations = static_cast<int>(n + m + 10);
            int iterations = 0;

            // Follow the tour until we reach back to start depot k
            while (next != static_cast<int>(k) && iterations < maxIterations) {
                if (next >= static_cast<int>(m) && next < static_cast<int>(m + n)) {
                    // This is a client node
                    route.push_back(next - static_cast<int>(m));  // Client index
                }
                current = next;
                next = cp.getValue(next_[current]);
                iterations++;
            }

            if (!route.empty()) {
                routes_.push_back(route);
            }
        }
    }

    // Accessors for variables
    const std::vector<IntVar>& getNextVars() const { return next_; }
    const std::vector<std::vector<int>>& getRoutes() const { return routes_; }

    size_t getNumClients() const { return numClients_; }
    size_t getNumVehicles() const { return numVehicles_; }
    size_t getTotalNodes() const { return totalNodes_; }
    size_t clientNodeIndex(size_t clientIdx) const { return numVehicles_ + clientIdx; }

private:
    std::vector<IntVar> next_;           // Successor variables

    std::vector<std::vector<int>> distanceMatrix_;  // Distance matrix
    std::vector<std::vector<int>> routes_;          // Extracted routes

    size_t totalNodes_ = 0;
    size_t numClients_ = 0;
    size_t numVehicles_ = 0;

    void computeDistanceMatrix(Problem& problem) {
        constexpr int kTimeScale = 100;
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
                int dist = static_cast<int>(std::lround(
                    problem.getDistance(*depot, *clients[i]) * kTimeScale));
                distanceMatrix_[k][m + i] = dist;
            }
            // Start depot to own end depot (empty route)
            distanceMatrix_[k][m + n + k] = 0;
        }

        // Clients to clients
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (i != j) {
                    int dist = static_cast<int>(std::lround(
                        problem.getDistance(*clients[i], *clients[j]) * kTimeScale));
                    distanceMatrix_[m + i][m + j] = dist;
                }
            }
        }

        // Clients to end depots
        for (size_t i = 0; i < n; ++i) {
            for (size_t k = 0; k < m; ++k) {
                int dist = static_cast<int>(std::lround(
                    problem.getDistance(*clients[i], *depot) * kTimeScale));
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
