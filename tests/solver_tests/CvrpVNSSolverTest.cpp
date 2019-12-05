//
// Created by Sohaib LAFIFI on 05/12/2019.
//

#include <cvrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/VNSSolver.hpp>
#include <fstream>
#include "gtest/gtest.h"

class CvrpLSSolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;
    CvrpLSSolverTest() : Test() {
    if (!nullstream.is_open())
        nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(CvrpLSSolverTest, solve){
     vrp::VNSSolver<cvrp::Reader> vnsSolver("../../../data/CVRP/toy.vrp", this->nullstream);
     EXPECT_FALSE(dynamic_cast<vrp::Problem*>(vnsSolver.getProblem())->clients.empty());
     EXPECT_EQ(dynamic_cast<vrp::Problem*>(vnsSolver.getProblem())->clients.size(), 5);
     bool solution_found = vnsSolver.solve();
     EXPECT_TRUE(solution_found);
     EXPECT_GT(vnsSolver.getSolution()->getCost(), 190);
}