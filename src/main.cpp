#include <iostream>
#include <utilities/GetOpt_pp.hpp>
#include <cvrp/Reader.hpp>
#ifdef CPLEX
#include <core/solvers/MIPSolver.hpp>
#endif
#include <vrp/solvers/LSSolver.hpp>

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

    vrp::LSSolver<cvrp::Reader> lsSolver(inputFile);
    lsSolver.solve(timeout);
    return EXIT_SUCCESS;
}