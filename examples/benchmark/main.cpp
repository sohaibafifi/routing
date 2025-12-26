/**
 * @file main.cpp
 * @brief Benchmark CLI for Composable Routing Framework
 *
 * Usage:
 *   ./benchmark --instance R101.txt --solver ga --timeout 60 --variant cvrptw
 *
 * Outputs JSON results to stdout for easy parsing.
 *
 * @author Sohaib Lafifi
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>

// Core includes
#include "core/PluginRegistry.hpp"
#include "plugins/PluginBundle.hpp"

// Attribute includes
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"

// Argparse
#include "argparse/argparse.hpp"

using namespace routing;

/**
 * @brief Customer data from Solomon instance
 */
struct CustomerData {
    int id;
    double x, y;
    int demand;
    double readyTime, dueDate;
    double serviceTime;
};

/**
 * @brief Parsed Solomon instance
 */
struct SolomonInstance {
    std::string name;
    int numVehicles;
    int capacity;
    CustomerData depot;
    std::vector<CustomerData> customers;
};

/**
 * @brief Parse Solomon CVRPTW format instance file
 */
SolomonInstance parseSolomonFile(const std::string& filepath) {
    SolomonInstance instance;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    std::string line;

    // Read instance name (first line)
    std::getline(file, line);
    instance.name = line;

    // Skip empty lines and find VEHICLE section
    while (std::getline(file, line)) {
        if (line.find("VEHICLE") != std::string::npos) {
            break;
        }
    }

    // Skip header line (NUMBER CAPACITY)
    std::getline(file, line);

    // Read vehicle info
    std::getline(file, line);
    std::istringstream vehicleStream(line);
    vehicleStream >> instance.numVehicles >> instance.capacity;

    // Skip to CUSTOMER section
    while (std::getline(file, line)) {
        if (line.find("CUSTOMER") != std::string::npos || line.find("CUST") != std::string::npos) {
            break;
        }
    }

    // Skip header lines
    std::getline(file, line); // CUST NO. XCOORD. etc.

    // Read customer data
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        CustomerData cust;

        if (ss >> cust.id >> cust.x >> cust.y >> cust.demand
               >> cust.readyTime >> cust.dueDate >> cust.serviceTime) {

            if (cust.id == 0) {
                instance.depot = cust;
            } else {
                instance.customers.push_back(cust);
            }
        }
    }

    return instance;
}

/**
 * @brief Create composable problem from Solomon instance
 */
Problem* createProblem(const SolomonInstance& instance) {
    auto* problem = new Problem();

    // Enable required attributes for CVRPTW
    problem->enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();

    // Setup depot
    auto* depot = problem->getComposableDepot();
    depot->addAttribute<GeoNode>(instance.depot.x, instance.depot.y);
    depot->addAttribute<Rendezvous>(instance.depot.readyTime, instance.depot.dueDate);

    // Add clients
    for (const auto& cust : instance.customers) {
        auto* client = problem->addClient(cust.id);
        client->addAttribute<GeoNode>(cust.x, cust.y);
        client->addAttribute<Consumer>(cust.demand);
        client->addAttribute<Rendezvous>(cust.readyTime, cust.dueDate);
        client->addAttribute<ServiceQuery>(cust.serviceTime);
    }

    // Add vehicles
    for (int k = 0; k < instance.numVehicles; ++k) {
        auto* vehicle = problem->addVehicle(k);
        vehicle->addAttribute<Stock>(instance.capacity);
    }

    return problem;
}

/**
 * @brief Output JSON result
 */
void outputJson(
    const std::string& instance,
    const std::string& solver,
    const std::string& variant,
    double objective,
    double timeSeconds,
    int numVehicles,
    int numClients,
    const std::string& status,
    const std::string& errorMessage = ""
) {
    std::cout << "{" << std::endl;
    std::cout << "  \"instance\": \"" << instance << "\"," << std::endl;
    std::cout << "  \"solver\": \"" << solver << "\"," << std::endl;
    std::cout << "  \"variant\": \"" << variant << "\"," << std::endl;
    std::cout << "  \"objective\": " << std::fixed << std::setprecision(2) << objective << "," << std::endl;
    std::cout << "  \"time_seconds\": " << std::fixed << std::setprecision(3) << timeSeconds << "," << std::endl;
    std::cout << "  \"num_vehicles\": " << numVehicles << "," << std::endl;
    std::cout << "  \"num_clients\": " << numClients << "," << std::endl;
    std::cout << "  \"status\": \"" << status << "\"";
    if (!errorMessage.empty()) {
        std::cout << "," << std::endl;
        std::cout << "  \"error\": \"" << errorMessage << "\"";
    }
    std::cout << std::endl << "}" << std::endl;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("benchmark", "1.0");

    program.add_argument("-i", "--instance")
        .required()
        .help("Path to instance file (Solomon format)");

    program.add_argument("-s", "--solver")
        .default_value(std::string("ga"))
        .help("Solver to use: ga, vns, ma, pso, ls, mip, cp (default: ga)");

    program.add_argument("-t", "--timeout")
        .default_value(60)
        .scan<'i', int>()
        .help("Timeout in seconds (default: 60)");

    program.add_argument("-v", "--variant")
        .default_value(std::string("cvrptw"))
        .help("Problem variant (default: cvrptw)");

    program.add_argument("--verbose")
        .default_value(false)
        .implicit_value(true)
        .help("Verbose output");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string instancePath = program.get<std::string>("--instance");
    std::string solverName = program.get<std::string>("--solver");
    int timeout = program.get<int>("--timeout");
    std::string variant = program.get<std::string>("--variant");
    bool verbose = program.get<bool>("--verbose");

    // Extract instance name from path
    std::string instanceName = instancePath;
    size_t lastSlash = instancePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        instanceName = instancePath.substr(lastSlash + 1);
    }
    size_t lastDot = instanceName.find_last_of('.');
    if (lastDot != std::string::npos) {
        instanceName = instanceName.substr(0, lastDot);
    }

    try {
        // Initialize plugin system
        if (verbose) {
            std::cerr << "Initializing plugins..." << std::endl;
        }

        auto& registry = PluginRegistry::instance();
        registerCorePlugins(registry);
        registry.initializeAll();

        // Parse instance
        if (verbose) {
            std::cerr << "Parsing instance: " << instancePath << std::endl;
        }

        SolomonInstance instance = parseSolomonFile(instancePath);

        if (verbose) {
            std::cerr << "Instance: " << instance.name << std::endl;
            std::cerr << "  Vehicles: " << instance.numVehicles << std::endl;
            std::cerr << "  Capacity: " << instance.capacity << std::endl;
            std::cerr << "  Customers: " << instance.customers.size() << std::endl;
        }

        // Create problem
        Problem* problem = createProblem(instance);

        // Create solver
        if (verbose) {
            std::cerr << "Creating solver: " << solverName << std::endl;
        }

        auto solver = registry.createSolver(solverName, problem);
        if (!solver) {
            outputJson(instanceName, solverName, variant, 0, 0, 0,
                      instance.customers.size(), "error",
                      "Solver not found: " + solverName);
            delete problem;
            return 1;
        }

        // Run solver
        if (verbose) {
            std::cerr << "Solving with timeout: " << timeout << "s" << std::endl;
        }

        auto startTime = std::chrono::high_resolution_clock::now();
        bool success = solver->solve(static_cast<double>(timeout));
        auto endTime = std::chrono::high_resolution_clock::now();

        double elapsedSeconds = std::chrono::duration<double>(endTime - startTime).count();

        // Get solution
        if (success) {
            auto* solution = solver->getSolution();
            double objective = solver->getObjectiveValue();
            int numRoutes = solution ? solution->size() : 0;

            outputJson(instanceName, solverName, variant,
                      objective, elapsedSeconds, numRoutes,
                      instance.customers.size(), "success");
        } else {
            outputJson(instanceName, solverName, variant,
                      std::numeric_limits<double>::infinity(), elapsedSeconds, 0,
                      instance.customers.size(), "no_solution");
        }

        delete problem;

    } catch (const std::exception& e) {
        outputJson(instanceName, solverName, variant,
                  0, 0, 0, 0, "error", e.what());
        return 1;
    }

    return 0;
}
