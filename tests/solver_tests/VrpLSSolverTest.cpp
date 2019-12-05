//
// Created by Sohaib LAFIFI on 05/12/2019.
//

#include <vrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/LSSolver.hpp>
#include <fstream>
#include "gtest/gtest.h"

class VrpLSSolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;
    VrpLSSolverTest() : Test() {
    if (!nullstream.is_open())
        nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(VrpLSSolverTest, solve){

     vrp::LSSolver<vrp::Reader> lsSolver("", this->nullstream);
     EXPECT_FALSE(dynamic_cast<vrp::Problem*>(lsSolver.getProblem())->clients.empty());
     bool solution_found = lsSolver.solve();
     EXPECT_TRUE(solution_found);
     EXPECT_EQ(lsSolver.getSolution()->getTour(1)->getNbClient(), 0);
     EXPECT_GT(lsSolver.getSolution()->getCost(), 0);
}