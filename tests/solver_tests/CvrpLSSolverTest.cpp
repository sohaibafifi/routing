// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.



#include <cvrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/LSSolver.hpp>
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
     vrp::LSSolver<cvrp::Reader> lsSolver("../../../data/CVRP/toy.vrp", this->nullstream);
     EXPECT_FALSE(dynamic_cast<vrp::Problem*>(lsSolver.getProblem())->clients.empty());
     EXPECT_EQ(dynamic_cast<vrp::Problem*>(lsSolver.getProblem())->clients.size(), 5);
     bool solution_found = lsSolver.solve();
     EXPECT_TRUE(solution_found);
     EXPECT_GT(lsSolver.getSolution()->getCost(), 190);
}