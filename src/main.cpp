#include <iostream>
#include <utilities/Utilities.hpp>
#include <utilities/GetOpt_pp.hpp>
#include <cvrp/data/Reader.hpp>
#include <core/solvers/MIPSolver.hpp>

int main(int argc, char **argv) {
    Utilities::GetOpt::GetOpt_pp ops(argc, argv);
    ops.exceptions_all();
    std::string inputFile;
    double timeout = 200;
    ops >> Utilities::GetOpt::Option( 'i' , "input", inputFile);
    ops >> Utilities::GetOpt::Option( 't' , "timeout", timeout, timeout);
    routing::MIPSolver<cvrp::Reader> mipSolver(inputFile);
    mipSolver.solve(timeout);
    return 0;
}