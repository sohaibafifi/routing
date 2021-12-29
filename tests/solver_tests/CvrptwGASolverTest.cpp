// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <cvrptw/Reader.hpp>
#include <vrp/solvers/GASolver.hpp>
#include <fstream>
#include "gtest/gtest.h"

class CvrptwGASolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;

    CvrptwGASolverTest() : Test() {
        if (!nullstream.is_open())
            nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(CvrptwGASolverTest, solve) {
    vrp::GASolver<cvrptw::Reader> gaSolver("data/CVRPTW/Solomon/5/c103.txt", this->nullstream);
    EXPECT_FALSE(dynamic_cast<vrp::Problem *>(gaSolver.getProblem())->clients.empty());
    EXPECT_EQ(dynamic_cast<vrp::Problem *>(gaSolver.getProblem())->clients.size(), 3);
    bool solution_found = gaSolver.solve();
    EXPECT_TRUE(solution_found);
}