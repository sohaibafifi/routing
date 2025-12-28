// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <examples/problems/cvrptw/Reader.hpp>
#include <core/PluginRegistry.hpp>
#include <plugins/PluginBundle.hpp>

#ifdef CPLEX_FOUND
#include <plugins/solvers/MIPSolverPlugin/MIPSolver.hpp>
#include <plugins/solvers/MIPSolverPlugin/CPLEXMIPBackend.hpp>
#endif

#include "libs/argparse/argparse.h"

int main(int argc, const char *argv[]) {
    argparse::ArgumentParser parser(argv[0], "CVRPTW Solver (Composable)");
    parser.add_argument("-i", "--input", "Instance File", true);
    parser.add_argument("-o", "--output", "Output File", false);
    parser.add_argument("-t", "--timeout", "Timeout in seconds", false);
    parser.enable_help();

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        parser.print_help();
        return EXIT_FAILURE;
    }

    if (parser.exists("help")) {
        parser.print_help();
        return EXIT_SUCCESS;
    }

    std::string inputFile = parser.get<std::string>("input");
    double timeout = parser.exists("timeout") ? parser.get<double>("timeout") : 200.0;

    std::filesystem::path outputFile;
    std::filesystem::path lpFile;
    if (!parser.exists("output")) {
        std::filesystem::path inputPath(inputFile);
        std::filesystem::path outputFolder = std::filesystem::path("output") / inputPath.parent_path();
        std::filesystem::create_directories(outputFolder);
        outputFile = outputFolder / (inputPath.stem().string() + ".result");
        lpFile = outputFolder / (inputPath.stem().string() + ".lp");
    } else {
        outputFile = parser.get<std::string>("output");
        lpFile = outputFile;
        lpFile += ".lp";
    }

    std::ofstream output(outputFile);

    routing::plugins::registerCorePlugins();
    auto& registry = routing::PluginRegistry::instance();
    registry.initializeAll();

#ifdef CPLEX_FOUND
    try {
        auto problem = composable::cvrptw::Reader().readFile(inputFile);
        routing::MIPSolver mipSolver(problem);

        // Export model via CPLEX backend
        if (auto* cplexBackend = dynamic_cast<routing::mip::CPLEXMIPBackend*>(&mipSolver.getBackend())) {
            cplexBackend->exportModel(lpFile.string());
        }

        bool solved = mipSolver.solve(timeout);

        if (output.is_open()) {
            output << problem->getName() << "\t"
                   << (solved ? "Solved" : "NotSolved") << "\t"
                   << mipSolver.getObjectiveValue() << "\t"
                   << mipSolver.getBackend().getObjectiveBound() << "\t"
                   << mipSolver.getBackend().getGap() << "\t"
                   << mipSolver.getBackend().getSolveTime() << std::endl;
        }
    } catch (IloCplex::Exception &exception) {
        std::cout << exception.getMessage() << std::endl;
    }
#else
    std::cout << "CPLEX not available. Skipping solve." << std::endl;
#endif

    registry.shutdownAll();
    return EXIT_SUCCESS;
}
