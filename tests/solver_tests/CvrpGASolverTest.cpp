// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


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