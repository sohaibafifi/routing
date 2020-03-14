// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <vrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/GASolver.hpp>
#include <fstream>
#include "gtest/gtest.h"

class VrpGASolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;
    VrpGASolverTest() : Test() {
    if (!nullstream.is_open())
        nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(VrpGASolverTest, solve){

     vrp::GASolver<vrp::Reader> gaSolver("", this->nullstream);
     EXPECT_FALSE(dynamic_cast<vrp::Problem*>(gaSolver.getProblem())->clients.empty());
     bool solution_found = gaSolver.solve();
     EXPECT_TRUE(solution_found);
     EXPECT_EQ(gaSolver.getSolution()->getTour(1)->getNbClient(), 0);
     EXPECT_GT(gaSolver.getSolution()->getCost(), 0);
}