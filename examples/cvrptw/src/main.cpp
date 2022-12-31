// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <iostream>
#include <cvrptw/Reader.hpp>
#include <core/solvers/MIPSolver.hpp>
#include "../../libs/argparse/argparse.h"


int main(int argc, const char *argv[]) {
    argparse::ArgumentParser parser(argv[0], "VRPTW Solver");
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
    double timeout = parser.exists("timeout") ? parser.get<double>("timeout") : 200;


    std::string output_file;
    if (! parser.exists("output")) {
        std::string output_folder = "output/" + std::filesystem::path(inputFile).parent_path().string();
        system((std::string("mkdir -p ") + output_folder).c_str());
        output_file = output_folder + "/" + std::filesystem::path(inputFile).filename().string() + ".result";
    }else{
        output_file = parser.get<std::string>("output");
    }

    std::ofstream output(output_file);


    try {
        routing::MIPSolver<cvrptw::Reader> mipSolver(inputFile);
        mipSolver.solve(timeout);
        mipSolver.save(output);

    } catch (IloCplex::Exception &exception) {
        std::cout << exception.getMessage() << std::endl;
    }
    return EXIT_SUCCESS;
}