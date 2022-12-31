//
// Created by Sohaib Lafifi on 31/12/2022.
//


#include "../ProblemTest.hpp"

#include <cvrptw/Reader.hpp>
#include <cvrptw/Problem.hpp>

#ifdef CPLEX_FOUND
#include <core/solvers/MIPSolver.hpp>


TEST_F(ProblemTest, solve) {
    int nbClients = 10;
    routing::MIPSolver<cvrptw::Reader> mipSolver("../../../data/CVRPTW/Solomon/" + std::to_string(nbClients) + "/c101.txt");
    EXPECT_FALSE(dynamic_cast<cvrptw::Problem *>(mipSolver.getProblem())->clients.empty());
    EXPECT_EQ(dynamic_cast<cvrptw::Problem *>(mipSolver.getProblem())->clients.size(), nbClients);
    bool solution_found = mipSolver.solve();
    EXPECT_TRUE(solution_found);

}
#endif