/* Copyright 2022, Gurobi Optimization, LLC */

/* This example reads an LP model from a file and solves it.
   If the model is infeasible or unbounded, the example turns off
   presolve and solves the model again. If the model is infeasible,
   the example computes an Irreducible Inconsistent Subsystem (IIS),
   and writes it to a file */

#include <filesystem>
#include <iostream>
#include <fstream>
#include <argparse/argparse.h>
#include <ilcp/cp.h>
#include <ilcplex/ilocplex.h>

using namespace std;


int main(int argc, const char *argv[]) {
    argparse::ArgumentParser parser(argv[0], "Gurobi Solver");
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

    IloEnv env;
    IloModel model(env);
    IloCplex cplex = IloCplex(env);
    cplex.importModel(model, inputFile.c_str());
    try {
         IloCP cp = IloCP(model);
    cp.solve();



    std::string output_file;
    if (!parser.exists("output")) {
        output_file = inputFile + ".result_cp";
    } else
        output_file = parser.get<std::string>("output");

    std::ofstream output(output_file);
    string name = std::filesystem::path(inputFile).replace_extension("").filename().string();

    output <<
           name
           << "\t" << cp.getStatus()
           << "\t" << cp.getObjValue()
           << "\t" << cp.getObjBound()
           << "\t" << cp.getObjGap()
           << "\t" << cp.getTime()
           << std::endl;
    output.close();
    }catch (IloCP::Exception::IloException & e) {
        std::cerr << "IloException exception caught: " << e << std::endl;

 }
    return 0;
}
