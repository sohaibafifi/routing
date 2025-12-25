// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <core/PluginRegistry.hpp>
#include <core/interfaces/IReader.hpp>
#include <core/interfaces/ISolver.hpp>
#include <core/Problem.hpp>

// Include plugin bundle (auto-registration via static registrars)
#include <plugins/PluginBundle.hpp>

#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    routing::plugins::registerCorePlugins();
    routing::PluginRegistry::instance().initializeAll();

    std::cout << "Available solvers:" << std::endl;
    for (const auto& name : routing::PluginRegistry::instance().availableSolvers()) {
        std::cout << "  - " << name << std::endl;
    }

    std::cout << "\nAvailable neighborhoods:" << std::endl;
    for (const auto& name : routing::PluginRegistry::instance().availableNeighborhoods()) {
        std::cout << "  - " << name << std::endl;
    }

    std::cout << "\nAvailable readers:" << std::endl;
    for (const auto& name : routing::PluginRegistry::instance().availableReaders()) {
        std::cout << "  - " << name << std::endl;
    }

    if (argc < 3) {
        std::cout << "\nUsage: plugin_demo <solver> <instance_path>" << std::endl;
        std::cout << "Note: metaheuristics (ga/vns) require generator configuration." << std::endl;
        routing::PluginRegistry::instance().shutdownAll();
        return 0;
    }

    std::string solverName = argv[1];
    std::string instancePath = argv[2];
    std::string extension = std::filesystem::path(instancePath).extension().string();

    try {
        auto reader = routing::PluginRegistry::instance().createReaderForExtension(extension);
        routing::Problem* problem = reader->readFile(instancePath);

        auto solver = routing::PluginRegistry::instance().createSolver(solverName, problem);
        solver->setDefaultConfiguration();
        solver->solve(10.0);

        std::cout << "\nBest cost: " << solver->getObjectiveValue() << std::endl;

        delete problem;
    } catch (const std::exception& e) {
        std::cerr << "\nPlugin demo failed: " << e.what() << std::endl;
        routing::PluginRegistry::instance().shutdownAll();
        return 1;
    }

    routing::PluginRegistry::instance().shutdownAll();
    return 0;
}
