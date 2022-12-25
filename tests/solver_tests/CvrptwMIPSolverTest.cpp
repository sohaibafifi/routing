// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <cvrptw/Reader.hpp>
#include <vrp/Problem.hpp>


#include <core/solvers/MIPSolver.hpp>


#include <fstream>
#include "gtest/gtest.h"

class CvrptwMIPSolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;

    CvrptwMIPSolverTest() : Test() {
        if (!nullstream.is_open())
            nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(CvrptwMIPSolverTest, solve) {
    routing::MIPSolver<cvrptw::Reader> mipSolver("data/CVRPTW/Solomon/100/c103.txt");
    EXPECT_FALSE(dynamic_cast<vrp::Problem *>(mipSolver.getProblem())->clients.empty());
    EXPECT_EQ(dynamic_cast<vrp::Problem *>(mipSolver.getProblem())->clients.size(), 100);
    bool solution_found = mipSolver.solve();
    EXPECT_TRUE(solution_found);

}