#include <iostream>
#include <fstream>
#include "problems/ccvrp/ccvrp.hpp"
#include "problems/cvrptw/cvrptw.hpp"
#include "problems/toptw/toptw.hpp"
#include "problems/vrptwsyn/vrptwsyn.hpp"
#include "solvers/MIPSolver.hpp"
#include "solvers/LSSolver.hpp"
#pragma GCC diagnostic push
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma clang diagnostic ignored "-Wdeprecated"
#include "getopt_pp.hpp"
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
using namespace GetOpt;
using namespace std;

#include "ConfigureCallbacksCall.h"
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
        std::string output;

        Config* config = new Config();

        std::ofstream out;

        ops >> Option( 'i' , "input", inputFile);
        ops >> Option( 't' , "timeout", timeout, timeout);

        //set output options
        //true: print in output file passed as argument
        //false: print in standard output
        config->setOutputStreamFile(true);
        if(config->isOutputStreamFile()){
            ops >> Option( 'o' , "output", output);
            out.open (output,std::ios::out);

        }else{
            out.copyfmt(std::cout);
            out.clear(std::cout.rdstate());
            out.basic_ios<char>::rdbuf(std::cout.rdbuf());
        }

        //all solvers have default argument for output as std::cout
        //so if no output is passed, default stream would be std::cout

        if (inputFile.find("/CVRP/") != std::string::npos)
            CVRP::LSSolver<CVRP::Reader>(inputFile,out).solve(timeout);
        if (inputFile.find("/CVRPTW/") != std::string::npos)
            //routing::MIPSolver<CVRPTW::Reader>(inputFile,*config).solve(timeout);
            CVRPTW::LSSolver<CVRPTW::Reader>(inputFile).solve(timeout);
        if (inputFile.find("/TOPTW/") != std::string::npos)
            routing::MIPSolver<TOPTW::Reader>(inputFile, *config,out).solve(timeout);
        if (inputFile.find("/VRPTWSyn/") != std::string::npos)
            routing::MIPSolver<VRPTWSyn::Reader>(inputFile, *config, out).solve(timeout);

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
