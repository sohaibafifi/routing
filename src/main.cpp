#include <iostream>
#include <utilities/GetOpt_pp.hpp>
#include <vrp/Reader.hpp>
#include <core/solvers/MIPSolver.hpp>
#include <vrp/solvers/LSSolver.hpp>
#include <vrp/routines/operators/Generator.hpp>

int main(int argc, char **argv) {
    Utilities::GetOpt::GetOpt_pp ops(argc, argv);
    ops.exceptions_all();
    std::string inputFile;
    double timeout = 200;
    ops >> Utilities::GetOpt::Option('i', "input", inputFile);
    ops >> Utilities::GetOpt::Option('t', "timeout", timeout, timeout);
//    try {
//        routing::MIPSolver<vrp::Reader> mipSolver(inputFile);
//        mipSolver.solve(timeout);
//    }catch (IloCplex::Exception & exception){
//        std::cout << exception.getMessage() << std::endl;
//    }



    vrp::LSSolver<vrp::Reader> lsSolver(inputFile);
    lsSolver.solve(timeout);
    return EXIT_SUCCESS;
}