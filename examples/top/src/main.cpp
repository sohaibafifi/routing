// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <iostream>
#include <top/Reader.hpp>
#include <core/solvers/MIPSolver.hpp>
#include "../../libs/argparse/argparse.h"


int main(int argc, const char *argv[]) {
    argparse::ArgumentParser parser(argv[0], "TOP Solver");
    parser.add_argument("-i", "--input", "Instance File", true);
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


    auto inputFile = parser.get<std::string>("input");
    double timeout = parser.exists("timeout") ? parser.get<double>("timeout") : 200;


    try {
        routing::MIPSolver<top::Reader> mipSolver(inputFile);
        mipSolver.solve(timeout);
    } catch (IloCplex::Exception &exception) {
        std::cout << exception.getMessage() << std::endl;
    }
    return EXIT_SUCCESS;
}