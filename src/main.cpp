#include <iostream>
#include <utilities/Utilities.hpp>
#include <utilities/GetOpt_pp.hpp>
#include <vrp/data/Reader.hpp>
#include <core/solvers/MIPSolver.hpp>

int main(int argc, char **argv) {
    Utilities::GetOpt::GetOpt_pp ops(argc, argv);
    ops.exceptions_all();

    routing::MIPSolver<vrp::Reader> mipSolver("");
    mipSolver.solve();
    return 0;
}