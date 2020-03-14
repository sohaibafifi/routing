// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <vrp/Reader.hpp>
#include <vrp/Problem.hpp>
#include <vrp/solvers/VNSSolver.hpp>
#include <fstream>
#include "gtest/gtest.h"

class VrpVNSSolverTest : public ::testing::Test {
public :
    std::ofstream nullstream;
    VrpVNSSolverTest() : Test() {
    if (!nullstream.is_open())
        nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};

TEST_F(VrpVNSSolverTest, solve){

     vrp::VNSSolver<vrp::Reader> vnsSolver("", this->nullstream);
     EXPECT_FALSE(dynamic_cast<vrp::Problem*>(vnsSolver.getProblem())->clients.empty());
     bool solution_found = vnsSolver.solve();
     EXPECT_TRUE(solution_found);
     EXPECT_EQ(vnsSolver.getSolution()->getTour(1)->getNbClient(), 0);
     EXPECT_GT(vnsSolver.getSolution()->getCost(), 0);
}