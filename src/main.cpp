#include <iostream>
#include "problems/ccvrp/ccvrp.hpp"
#include "problems/cvrptw/cvrptw.hpp"
#include "problems/toptw/toptw.hpp"
#include "problems/vrptwsyn/vrptwsyn.hpp"
#include "Solver.hpp"
#pragma GCC diagnostic push
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma clang diagnostic ignored "-Wdeprecated"
#include "getopt_pp.hpp"
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
using namespace GetOpt;
using namespace std;
/*!
 * Main function with a test run
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv)
{
    GetOpt_pp ops(argc, argv);
    ops.exceptions_all();
    try{
        std::string inputFile;
        double timeout = 200;
        ops >> Option( 'i' , "input", inputFile);
        ops >> Option( 't' , "timeout", timeout, timeout);

        if (inputFile.find("/CVRP/") != std::string::npos)
            Solver::create<CVRP::Reader>(inputFile, timeout)->solve();
        if (inputFile.find("/CVRPTW/") != std::string::npos)
            Solver::create<CVRPTW::Reader>(inputFile, timeout)->solve();
        if (inputFile.find("/TOPTW/") != std::string::npos)
            Solver::create<TOPTW::Reader>(inputFile, timeout)->solve();
        if (inputFile.find("/VRPTWSyn/") != std::string::npos)
            Solver::create<VRPTWSyn::Reader>(inputFile, timeout)->solve();

        return EXIT_SUCCESS;

    }catch(IloException & x)
    {
        cerr << x.getMessage() << endl;
    }
    catch(TooManyOptionsEx)
    {
        cerr << "You specified more options than those I understand" << endl;
    }
    catch(GetOptEx)
    {
        cerr << "Invalid options. Try again"<< endl;
    }
    return EXIT_FAILURE;
}
