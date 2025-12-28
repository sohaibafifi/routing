//
// Created by Sohaib Lafifi on 31/12/2022.
//


#include "../ProblemTest.hpp"

#include <examples/problems/cvrptw/Reader.hpp>
#include <examples/problems/cvrptw/Problem.hpp>
#include <core/PluginRegistry.hpp>
#include <plugins/PluginBundle.hpp>

#ifdef CPLEX_FOUND
#include <plugins/solvers/MIPSolverPlugin/MIPSolver.hpp>


TEST_F(ProblemTest, solve) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();
    routing::plugins::registerCorePlugins();
    registry.initializeAll();

    int nbClients = 10;
    auto problem = composable::cvrptw::Reader().readFile("../../../data/CVRPTW/Solomon/" + std::to_string(nbClients) + "/c101.txt");
    {
        routing::MIPSolver mipSolver(problem);
        auto* typed = dynamic_cast<composable::cvrptw::Problem *>(mipSolver.getProblem());
        ASSERT_NE(typed, nullptr);
        EXPECT_FALSE(typed->getClients().empty());
        EXPECT_EQ(typed->numClients(), static_cast<size_t>(nbClients));
        bool solution_found = mipSolver.solve(60.0);
        EXPECT_TRUE(solution_found);
    }

    registry.shutdownAll();
    registry.clear();
}
#endif
