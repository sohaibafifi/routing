//
// Created by Sohaib Lafifi on 31/12/2022.
//


#include "../ProblemTest.hpp"

#include <cvrpstw/Reader.hpp>
#include <cvrpstw/Problem.hpp>

#ifdef CPLEX_FOUND
#include <core/solvers/MIPSolver.hpp>


TEST_F(ProblemTest, solve) {
    int nbClients = 10;
    routing::MIPSolver<cvrpstw::Reader> mipSolver("../../../data/CVRPTW/Solomon/" + std::to_string(nbClients) + "/c101.txt");
    EXPECT_FALSE(dynamic_cast<cvrpstw::Problem *>(mipSolver.getProblem())->clients.empty());
    EXPECT_EQ(dynamic_cast<cvrpstw::Problem *>(mipSolver.getProblem())->clients.size(), nbClients);
    bool solution_found = mipSolver.solve();
    EXPECT_TRUE(solution_found);

}
#endif