// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <iostream>
#include <utilities/GetOpt_pp.hpp>
#include <vrp/Reader.hpp>

#ifdef CPLEX
#include <core/solvers/MIPSolver.hpp>
#endif

#include <vrp/solvers/LSSolver.hpp>
#include <vrp/solvers/MASolver.hpp>

int main(int argc, char **argv) {
    Utilities::GetOpt::GetOpt_pp ops(argc, argv);
    ops.exceptions_all();
    std::string inputFile;
    double timeout = 200;
    ops >> Utilities::GetOpt::Option('i', "input", inputFile);
    ops >> Utilities::GetOpt::Option('t', "timeout", timeout, timeout);
#ifdef CPLEX
    try {
        routing::MIPSolver<cvrp::Reader> mipSolver(inputFile);
        mipSolver.solve(timeout);
    }catch (IloCplex::Exception & exception){
        std::cout << exception.getMessage() << std::endl;
    }
#endif

    vrp::MASolver<vrp::Reader> maSolver(inputFile);
    maSolver.solve(timeout);
    return EXIT_SUCCESS;
}