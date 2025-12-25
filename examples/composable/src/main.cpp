// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

/**
 * @file main.cpp
 * @brief Example demonstrating the Composable Attribute System
 *
 * This example shows how to:
 * 1. Create a ComposableProblem
 * 2. Enable attributes at runtime
 * 3. Add entities (clients, vehicles, depots) with attributes
 * 4. Solve using the MIP solver with auto-generated constraints
 *
 * The composable system replaces the inheritance hierarchy (VRP → CVRP → CVRPTW)
 * with runtime attribute composition, enabling:
 * - Mix-and-match of problem features
 * - Custom problem variants without new classes
 * - Dynamic problem configuration
 */

#include <iostream>
#include <iomanip>

// Core composable system
#include <plugins/attributes/ComposableCorePlugin/ComposableProblem.hpp>
#include <plugins/attributes/ComposableCorePlugin/ComposableEntity.hpp>
#include <core/PluginRegistry.hpp>

// Attributes
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>
#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>
#include <plugins/attributes/TimeWindowPlugin/Rendezvous.hpp>
#include <plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp>
#include <plugins/attributes/ProfitPlugin/Profiter.hpp>

// Plugin bundle (auto-registers all core plugins)
#include <plugins/PluginBundle.hpp>

// MIP Solver
#include <plugins/solvers/MIPSolverPlugin/MIPSolver.hpp>

using namespace routing;
using namespace routing::attributes;

/**
 * @brief Create a simple CVRPTW instance using composable attributes
 */
void exampleCVRPTW() {
    std::cout << "\n=== Example: CVRPTW via Composable Attributes ===" << std::endl;

    ComposableProblem problem;

    // Enable attributes for CVRPTW problem
    // This activates: RoutingConstraintGenerator, CapacityConstraintGenerator, TimeWindowConstraintGenerator
    problem.enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();

    std::cout << "Enabled attributes: GeoNode, Consumer, Stock, Rendezvous, ServiceQuery" << std::endl;
    std::cout << "Active generators: " << problem.getActiveGenerators().size() << std::endl;
    for (auto* gen : problem.getActiveGenerators()) {
        std::cout << "  - " << gen->name() << " (priority: " << gen->priority() << ")" << std::endl;
    }

    // Add depot at origin with time window [0, 1000]
    auto* depot = problem.addDepot(0);
    depot->addAttribute<GeoNode>(0.0, 0.0);
    depot->addAttribute<Rendezvous>(0, 1000);

    // Add vehicles with capacity 100
    for (int v = 0; v < 3; ++v) {
        auto* vehicle = problem.addVehicle(v);
        vehicle->addAttribute<Stock>(100);
    }

    // Add clients with locations, demands, time windows, and service times
    struct ClientData {
        unsigned id;
        double x, y;
        int demand;
        int twOpen, twClose;
        int serviceTime;
    };

    std::vector<ClientData> clientData = {
        {1, 10.0, 20.0, 15, 50, 200, 10},
        {2, 25.0, 15.0, 20, 100, 300, 15},
        {3, 30.0, 35.0, 10, 150, 400, 10},
        {4, 15.0, 40.0, 25, 200, 500, 20},
        {5, 40.0, 25.0, 30, 100, 350, 15},
    };

    for (const auto& data : clientData) {
        auto* client = problem.addClient(data.id);
        client->addAttribute<GeoNode>(data.x, data.y);
        client->addAttribute<Consumer>(data.demand);
        client->addAttribute<Rendezvous>(data.twOpen, data.twClose);
        client->addAttribute<ServiceQuery>(data.serviceTime);
    }

    std::cout << "\nProblem created:" << std::endl;
    std::cout << "  Clients: " << problem.numClients() << std::endl;
    std::cout << "  Vehicles: " << problem.numVehicles() << std::endl;

    // Display client info
    std::cout << "\nClients:" << std::endl;
    std::cout << std::setw(4) << "ID" << std::setw(8) << "X" << std::setw(8) << "Y"
              << std::setw(8) << "Demand" << std::setw(12) << "TW" << std::setw(10) << "Service" << std::endl;
    for (auto* client : problem.getComposableClients()) {
        auto* geo = client->tryGetAttribute<GeoNode>();
        auto* cons = client->tryGetAttribute<Consumer>();
        auto* tw = client->tryGetAttribute<Rendezvous>();
        auto* svc = client->tryGetAttribute<ServiceQuery>();

        std::cout << std::setw(4) << client->getID()
                  << std::setw(8) << (geo ? geo->getX() : 0)
                  << std::setw(8) << (geo ? geo->getY() : 0)
                  << std::setw(8) << (cons ? cons->getDemand() : 0)
                  << std::setw(6) << "[" << (tw ? tw->getTwOpen() : 0) << ","
                  << (tw ? tw->getTwClose() : 0) << "]"
                  << std::setw(10) << (svc ? svc->getService() : 0)
                  << std::endl;
    }

#ifdef CPLEX_FOUND
    std::cout << "\nSolving with MIP (timeout: 10s)..." << std::endl;

    try {
        // Create a new problem for solving (MIPSolver takes ownership)
        auto* solvableProblem = new ComposableProblem();
        solvableProblem->setName("CVRPTW-Composable");
        solvableProblem->enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();

        // Add depot
        auto* solveDepot = solvableProblem->addDepot(0);
        solveDepot->addAttribute<GeoNode>(0.0, 0.0);
        solveDepot->addAttribute<Rendezvous>(0, 1000);

        // Add vehicles
        for (int v = 0; v < 3; ++v) {
            auto* vehicle = solvableProblem->addVehicle(v);
            vehicle->addAttribute<Stock>(100);
        }

        // Add clients (same as before)
        for (const auto& data : clientData) {
            auto* c = solvableProblem->addClient(data.id);
            c->addAttribute<GeoNode>(data.x, data.y);
            c->addAttribute<Consumer>(data.demand);
            c->addAttribute<Rendezvous>(data.twOpen, data.twClose);
            c->addAttribute<ServiceQuery>(data.serviceTime);
        }

        // Solve with MIP
        routing::MIPSolver solver(solvableProblem);
        bool solved = solver.solve(10.0);  // 10 second timeout

        if (solved) {
            std::cout << "  Solution found!" << std::endl;
        } else {
            std::cout << "  No optimal solution within timeout." << std::endl;
        }
    } catch (const IloException& e) {
        std::cout << "CPLEX Exception: " << e.getMessage() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
#else
    std::cout << "\nCPLEX not available. Skipping solve." << std::endl;
#endif
}

/**
 * @brief Create a TOP (Team Orienteering Problem) instance
 */
void exampleTOP() {
    std::cout << "\n=== Example: TOP via Composable Attributes ===" << std::endl;

    ComposableProblem problem;

    // Enable attributes for TOP problem (maximize profit, respect capacity)
    problem.enableAttributes<GeoNode, Consumer, Stock, Profiter>();

    std::cout << "Enabled attributes: GeoNode, Consumer, Stock, Profiter" << std::endl;
    std::cout << "Active generators: " << problem.getActiveGenerators().size() << std::endl;
    for (auto* gen : problem.getActiveGenerators()) {
        std::cout << "  - " << gen->name() << std::endl;
    }

    // Add depot
    auto* depot = problem.addDepot(0);
    depot->addAttribute<GeoNode>(0.0, 0.0);

    // Add vehicles
    for (int v = 0; v < 2; ++v) {
        auto* vehicle = problem.addVehicle(v);
        vehicle->addAttribute<Stock>(50);  // Max capacity per vehicle
    }

    // Add clients with profits
    struct ClientData {
        unsigned id;
        double x, y;
        int demand;
        double profit;
    };

    std::vector<ClientData> clientData = {
        {1, 10.0, 10.0, 5, 100.0},
        {2, 20.0, 15.0, 8, 150.0},
        {3, 15.0, 25.0, 6, 80.0},
        {4, 30.0, 20.0, 10, 200.0},
        {5, 25.0, 30.0, 7, 120.0},
        {6, 35.0, 10.0, 12, 180.0},
    };

    for (const auto& data : clientData) {
        auto* client = problem.addClient(data.id);
        client->addAttribute<GeoNode>(data.x, data.y);
        client->addAttribute<Consumer>(data.demand);
        client->addAttribute<Profiter>(data.profit);
    }

    std::cout << "\nProblem created:" << std::endl;
    std::cout << "  Clients: " << problem.numClients() << std::endl;
    std::cout << "  Vehicles: " << problem.numVehicles() << std::endl;

    // Display client info
    std::cout << "\nClients:" << std::endl;
    std::cout << std::setw(4) << "ID" << std::setw(8) << "X" << std::setw(8) << "Y"
              << std::setw(8) << "Demand" << std::setw(10) << "Profit" << std::endl;
    for (auto* client : problem.getComposableClients()) {
        auto* geo = client->tryGetAttribute<GeoNode>();
        auto* cons = client->tryGetAttribute<Consumer>();
        auto* prof = client->tryGetAttribute<Profiter>();

        std::cout << std::setw(4) << client->getID()
                  << std::setw(8) << (geo ? geo->getX() : 0)
                  << std::setw(8) << (geo ? geo->getY() : 0)
                  << std::setw(8) << (cons ? cons->getDemand() : 0)
                  << std::setw(10) << (prof ? prof->getProfit() : 0)
                  << std::endl;
    }
}

/**
 * @brief Demonstrate runtime attribute introspection
 */
void exampleIntrospection() {
    std::cout << "\n=== Example: Runtime Attribute Introspection ===" << std::endl;

    ComposableProblem problem;
    problem.enableAttributes<GeoNode, Consumer, Rendezvous>();

    auto* client = problem.addClient(1);
    client->addAttribute<GeoNode>(10.0, 20.0);
    client->addAttribute<Consumer>(15);
    // Note: Rendezvous enabled but not added to this client

    std::cout << "Client 1 attributes:" << std::endl;
    std::cout << "  Has GeoNode: " << (client->hasAttribute<GeoNode>() ? "yes" : "no") << std::endl;
    std::cout << "  Has Consumer: " << (client->hasAttribute<Consumer>() ? "yes" : "no") << std::endl;
    std::cout << "  Has Rendezvous: " << (client->hasAttribute<Rendezvous>() ? "yes" : "no") << std::endl;
    std::cout << "  Has Stock: " << (client->hasAttribute<Stock>() ? "yes" : "no") << std::endl;

    // Safe attribute access
    if (auto* geo = client->tryGetAttribute<GeoNode>()) {
        std::cout << "  Location: (" << geo->getX() << ", " << geo->getY() << ")" << std::endl;
    }
    if (auto* cons = client->tryGetAttribute<Consumer>()) {
        std::cout << "  Demand: " << cons->getDemand() << std::endl;
    }

    // Get all attribute types for this entity
    auto attrTypes = client->getAttributeTypes();
    std::cout << "  Total attributes: " << attrTypes.size() << std::endl;
}

int main(int argc, const char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  Composable Attribute System Demo" << std::endl;
    std::cout << "========================================" << std::endl;

    routing::plugins::registerCorePlugins();
    routing::PluginRegistry::instance().initializeAll();

    // Run examples
    exampleCVRPTW();
    exampleTOP();
    exampleIntrospection();

    routing::PluginRegistry::instance().shutdownAll();

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Demo Complete" << std::endl;
    std::cout << "========================================" << std::endl;

    return EXIT_SUCCESS;
}
