//
// Created by Sohaib Lafifi on 31/12/2022.
//


#include "../ProblemTest.hpp"

#include <compsable/cvrpstw/Reader.hpp>
#include <compsable/cvrpstw/Problem.hpp>
#include <core/PluginRegistry.hpp>
#include <plugins/attributes/CapacityPlugin/CapacityPlugin.hpp>
#include <plugins/attributes/ComposableCorePlugin/ComposableCorePlugin.hpp>
#include <plugins/attributes/RoutingPlugin/RoutingPlugin.hpp>
#include <plugins/attributes/TimeWindowPlugin/SoftTimeWindowGenerator.hpp>

#ifdef CPLEX_FOUND
#include <plugins/solvers/MIPSolverPlugin/MIPSolver.hpp>


TEST_F(ProblemTest, solve) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();
    registry.registerPlugin(std::make_unique<routing::plugins::ComposableCorePlugin>());
    registry.registerPlugin(std::make_unique<routing::plugins::RoutingPlugin>());
    registry.registerPlugin(std::make_unique<routing::plugins::CapacityPlugin>());
    registry.initializeAll();
    if (!registry.hasGenerator("SoftTimeWindowGenerator")) {
        registry.registerGenerator(std::make_unique<routing::constraints::SoftTimeWindowGenerator>(1.0, 1.0));
    }

    int nbClients = 10;
    auto problem = compsable::cvrpstw::Reader().readFile("../../../data/CVRPTW/Solomon/" + std::to_string(nbClients) + "/c101.txt");
    {
        routing::MIPSolver mipSolver(problem, nullstream);
        auto* typed = dynamic_cast<compsable::cvrpstw::Problem *>(mipSolver.getProblem());
        ASSERT_NE(typed, nullptr);
        EXPECT_FALSE(typed->getClients().empty());
        EXPECT_EQ(typed->numClients(), static_cast<size_t>(nbClients));
        bool solution_found = mipSolver.solve();
        EXPECT_TRUE(solution_found);
    }

    registry.shutdownAll();
    registry.clear();
}
#endif
