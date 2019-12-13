//
// Created by Sohaib LAFIFI on 05/12/2019.
//

#include <vrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/MASolver.hpp>
#include <fstream>
#include "gtest/gtest.h"

class VrpMASolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;
    VrpMASolverTest() : Test() {
    if (!nullstream.is_open())
        nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(VrpMASolverTest, solve){

     vrp::MASolver<vrp::Reader> maSolver("", this->nullstream);
     EXPECT_FALSE(dynamic_cast<vrp::Problem*>(maSolver.getProblem())->clients.empty());
     bool solution_found = maSolver.solve();
     EXPECT_TRUE(solution_found);
     EXPECT_EQ(maSolver.getSolution()->getTour(1)->getNbClient(), 0);
     EXPECT_GT(maSolver.getSolution()->getCost(), 0);
}