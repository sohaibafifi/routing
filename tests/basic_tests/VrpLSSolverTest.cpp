//
// Created by Sohaib LAFIFI on 05/12/2019.
//

#include <vrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/LSSolver.hpp>
#include "gtest/gtest.h"

class VrpLSSolverTest : public ::testing::Test {
public :
    VrpLSSolverTest() : Test() {

    }
};

TEST_F(VrpLSSolverTest, solve){
     vrp::LSSolver<vrp::Reader> lsSolver("");
     lsSolver.solve();
     EXPECT_EQ(lsSolver.getSolution()->getTour(1)->getNbClient(),
             0);
     EXPECT_GT(lsSolver.getSolution()->getCost(),
             0);
}