// Copyright (c) 2021. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <cvrptw/Reader.hpp>
#include <cvrptw/Problem.hpp>
#include <vrp/solvers/MASolver.hpp>
#include <fstream>
#include <vrp/solvers/GASolver.hpp>
#include "gtest/gtest.h"

class CvrptwMASolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;
    CvrptwMASolverTest() : Test() {
    if (!nullstream.is_open())
        nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(CvrptwMASolverTest, solve){
    vrp::MASolver<cvrptw::Reader> maSolver("data/CVRPTW/Solomon/100/c103.txt");
    EXPECT_FALSE(dynamic_cast<vrp::Problem *>(maSolver.getProblem())->clients.empty());
    EXPECT_EQ(dynamic_cast<vrp::Problem *>(maSolver.getProblem())->clients.size(), 100);
    bool solution_found = maSolver.solve();
    EXPECT_TRUE(solution_found);
}