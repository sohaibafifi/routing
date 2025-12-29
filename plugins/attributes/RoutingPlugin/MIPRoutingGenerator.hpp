// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IMIPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"

#include <vector>
#include <cmath>

namespace routing {
namespace mip {
namespace generators {

/**
 * @brief MIP Routing constraint generator
 *
 * Implements arc-based VRP formulation:
 * - Binary arc variables x[i][j] = 1 if arc (i,j) is used
 * - Flow conservation: each client visited exactly once
 * - Vehicle limit: at most m vehicles leave depot
 * - MTZ subtour elimination constraints
 * - Distance-based objective
 *
 * Node indices:
 * - 0: depot
 * - 1 to n: clients
 */
class MIPRoutingGenerator : public IMIPConstraintGenerator {
public:
    std::string name() const override {
        return "MIPRoutingGenerator";
    }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return { std::type_index(typeid(attributes::GeoNode)) };
    }

    int priority() const override {
        return 10;  // Runs first - provides base variables
    }

    void addVariables(IMIPBackend& mip, Problem& problem) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();
        auto depots = problem.getComposableDepots();

        if (depots.empty() || clients.empty()) return;

        size_t n = clients.size();
        size_t m = vehicles.size();

        numClients_ = n;
        numVehicles_ = m;

        // Create arc variables: x[i][j] = 1 if arc (i,j) is used
        // Index 0 is depot, indices 1..n are clients
        arcVars_.resize(n + 1);
        for (size_t i = 0; i <= n; ++i) {
            arcVars_[i].resize(n + 1);
            for (size_t j = 0; j <= n; ++j) {
                if (i != j) {
                    std::string varName = "x_" + std::to_string(i) + "_" + std::to_string(j);
                    arcVars_[i][j] = mip.newBoolVar(varName);
                }
            }
        }

        // Create MTZ variables for subtour elimination
        // u[i] represents the position of client i in its tour
        mtzVars_.resize(n + 1);
        for (size_t i = 1; i <= n; ++i) {
            mtzVars_[i] = mip.newIntVar(1, static_cast<int>(n), "u_" + std::to_string(i));
        }
    }

    void addConstraints(IMIPBackend& mip, Problem& problem) override {
        size_t n = numClients_;
        size_t m = numVehicles_;

        if (n == 0 || m == 0) return;

        // Flow conservation: each client visited exactly once
        for (size_t j = 1; j <= n; ++j) {
            // Sum of incoming arcs = 1
            LinearExpr inFlow;
            for (size_t i = 0; i <= n; ++i) {
                if (i != j) {
                    inFlow.addTerm(arcVars_[i][j]);
                }
            }
            mip.addEqual(inFlow, 1.0, "in_" + std::to_string(j));

            // Sum of outgoing arcs = 1
            LinearExpr outFlow;
            for (size_t k = 0; k <= n; ++k) {
                if (j != k) {
                    outFlow.addTerm(arcVars_[j][k]);
                }
            }
            mip.addEqual(outFlow, 1.0, "out_" + std::to_string(j));
        }

        // Depot flow: vehicles leave and return
        LinearExpr depotOut, depotIn;
        for (size_t j = 1; j <= n; ++j) {
            depotOut.addTerm(arcVars_[0][j]);
            depotIn.addTerm(arcVars_[j][0]);
        }
        mip.addLessEqual(depotOut, static_cast<double>(m), "depot_out");
        mip.addEqual(depotOut, depotIn, "depot_balance");

        // MTZ subtour elimination constraints
        // u[i] - u[j] + n * x[i][j] <= n - 1
        for (size_t i = 1; i <= n; ++i) {
            for (size_t j = 1; j <= n; ++j) {
                if (i != j) {
                    LinearExpr mtz;
                    mtz.addTerm(mtzVars_[i]);
                    mtz.addTerm(mtzVars_[j], -1.0);
                    mtz.addTerm(arcVars_[i][j], static_cast<double>(n));
                    mip.addLessEqual(mtz, static_cast<double>(n - 1),
                        "mtz_" + std::to_string(i) + "_" + std::to_string(j));
                }
            }
        }
    }

    void addObjectiveTerms(IMIPBackend& mip, Problem& problem, LinearExpr& expr) override {
        auto clients = problem.getComposableClients();
        auto depots = problem.getComposableDepots();

        if (depots.empty()) return;
        auto* depot = depots[0];

        size_t n = numClients_;

        // Minimize total distance
        for (size_t i = 0; i <= n; ++i) {
            for (size_t j = 0; j <= n; ++j) {
                if (i != j) {
                    double dist = 0.0;
                    if (i == 0 && j == 0) {
                        dist = 0.0;  // Depot to depot
                    } else if (i == 0) {
                        // From depot to client j-1
                        dist = problem.getDistanceEntity(depot, clients[j-1]);
                    } else if (j == 0) {
                        // From client i-1 to depot
                        dist = problem.getDistanceEntity(clients[i-1], depot);
                    } else {
                        // Between clients
                        dist = problem.getDistanceEntity(clients[i-1], clients[j-1]);
                    }
                    expr.addTerm(arcVars_[i][j], dist);
                }
            }
        }
    }

    void extractSolution(const IMIPBackend& mip, Problem& problem, Solution& solution) override {
        auto clients = problem.getComposableClients();
        auto vehicles = problem.getComposableVehicles();
        size_t n = numClients_;
        size_t m = numVehicles_;

        if (n == 0 || arcVars_.empty()) return;

        // Track which clients are visited
        std::vector<bool> visited(n + 1, false);
        visited[0] = true;  // Depot

        unsigned vehicleId = 0;

        // For each potential route starting from depot
        for (size_t startClient = 1; startClient <= n && vehicleId < m; ++startClient) {
            // Check if there's an arc from depot to this client
            if (mip.getBoolValue(arcVars_[0][startClient]) && !visited[startClient]) {
                // Start a new tour
                auto* tour = new Tour(&problem, vehicleId);

                // Follow the route
                size_t current = startClient;
                while (current != 0 && !visited[current]) {
                    visited[current] = true;
                    tour->_pushClient(clients[current - 1]);

                    // Find next node
                    size_t next = 0;
                    for (size_t j = 0; j <= n; ++j) {
                        if (j != current && mip.getBoolValue(arcVars_[current][j])) {
                            next = j;
                            break;
                        }
                    }
                    current = next;
                }

                if (tour->getNbClient() > 0) {
                    tour->update();
                    solution.pushTour(tour);
                    vehicleId++;
                } else {
                    delete tour;
                }
            }
        }

        solution.update();
    }

    // Accessors
    const std::vector<std::vector<BoolVar>>& getArcVars() const { return arcVars_; }
    const std::vector<IntVar>& getMTZVars() const { return mtzVars_; }
    size_t getNumClients() const { return numClients_; }
    size_t getNumVehicles() const { return numVehicles_; }

private:
    std::vector<std::vector<BoolVar>> arcVars_;  // Arc variables x[i][j]
    std::vector<IntVar> mtzVars_;                // MTZ variables u[i]
    size_t numClients_ = 0;
    size_t numVehicles_ = 0;
};

} // namespace generators
} // namespace mip
} // namespace routing
