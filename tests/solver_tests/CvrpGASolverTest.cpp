//
// Created by Sohaib LAFIFI on 05/12/2019.
//

#include <cvrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/GASolver.hpp>
#include <fstream>
#include "gtest/gtest.h"

class CvrpGASolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;
    CvrpGASolverTest() : Test() {
    if (!nullstream.is_open())
        nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(CvrpGASolverTest, solve){
     vrp::GASolver<cvrp::Reader> gaSolver("../../../data/CVRP/toy.vrp", this->nullstream);
     EXPECT_FALSE(dynamic_cast<vrp::Problem*>(gaSolver.getProblem())->clients.empty());
     EXPECT_EQ(dynamic_cast<vrp::Problem*>(gaSolver.getProblem())->clients.size(), 5);
     bool solution_found = gaSolver.solve();
     EXPECT_TRUE(solution_found);
     EXPECT_GT(gaSolver.getSolution()->getCost(), 190);
}