// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <cvrp/Reader.hpp>
#include <vrp/Problem.hpp>


#include <core/solvers/MIPSolver.hpp>


#include <fstream>
#include "gtest/gtest.h"

class CvrpMIPSolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;

    CvrpMIPSolverTest() : Test() {
        if (!nullstream.is_open())
            nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(CvrpMIPSolverTest, solve) {
    routing::MIPSolver<cvrp::Reader> mipSolver("data/CVRP/toy.vrp");
    EXPECT_FALSE(dynamic_cast<vrp::Problem *>(mipSolver.getProblem())->clients.empty());
    EXPECT_EQ(dynamic_cast<vrp::Problem *>(mipSolver.getProblem())->clients.size(), 5);
    bool solution_found = mipSolver.solve();
    EXPECT_TRUE(solution_found);

}