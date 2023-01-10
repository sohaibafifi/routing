/* Copyright 2022, Gurobi Optimization, LLC */

/* This example reads an LP model from a file and solves it.
   If the model is infeasible or unbounded, the example turns off
   presolve and solves the model again. If the model is infeasible,
   the example computes an Irreducible Inconsistent Subsystem (IIS),
   and writes it to a file */

#include "gurobi_c++.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <argparse/argparse.h>

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
    try {
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env, inputFile);
        model.set(GRB_DoubleParam_TimeLimit, timeout);

        model.optimize();

        std::string output_file;

        if (! parser.exists("output")) {
            output_file = inputFile + ".result_grb";
        }else
            output_file = parser.get<std::string>("output");

        std::ofstream output(output_file);
        string name = std::filesystem::path(inputFile).replace_extension("").filename().string();
        int optimstatus = model.get(GRB_IntAttr_Status);

        if (optimstatus == GRB_INF_OR_UNBD) {
            model.set(GRB_IntParam_Presolve, 0);
            model.optimize();
            optimstatus = model.get(GRB_IntAttr_Status);
        }

        if (optimstatus == GRB_OPTIMAL) {
            double objval = model.get(GRB_DoubleAttr_ObjVal);
            double objBound = model.get(GRB_DoubleAttr_ObjBound);
            double objGap = model.get(GRB_DoubleAttr_MIPGap);
            double cpu = model.get(GRB_DoubleAttr_Runtime);
            cout << "Optimal objective: " << objval << endl;
            output <<
                   name
                   << "\t" << "Optimal"
                   << "\t" << objval
                   << "\t" << objBound
                   << "\t" << objGap
                   << "\t" << cpu
                   << std::endl;
            output.close();
        } else if (optimstatus == GRB_INFEASIBLE) {
            cout << "Model is infeasible" << endl;


        } else if (optimstatus == GRB_UNBOUNDED) {
            cout << "Model is unbounded" << endl;
        } else {
            cout << "Optimization was stopped with status = "
                 << optimstatus << endl;
            double objval = model.get(GRB_DoubleAttr_ObjVal);
            double objBound = model.get(GRB_DoubleAttr_ObjBound);
            double objGap = model.get(GRB_DoubleAttr_MIPGap);
            double cpu = model.get(GRB_DoubleAttr_Runtime);
            cout << "Optimal objective: " << objval << endl;
            output <<
                   name
                   << "\t" << "Feasible"
                   << "\t" << objval
                   << "\t" << objBound
                   << "\t" << objGap
                   << "\t" << cpu
                   << std::endl;
            output.close();
        }

    } catch (GRBException &e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) {
        cout << "Error during optimization" << endl;
    }

    return 0;
}
