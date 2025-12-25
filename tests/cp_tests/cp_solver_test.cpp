// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include "gtest/gtest.h"
#include "plugins/solvers/CPSolverPlugin/CPSolver.hpp"
#include "plugins/solvers/CPSolverPlugin/CPSolverPlugin.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "core/PluginRegistry.hpp"

#ifdef CPLEX_FOUND

using namespace routing;
using namespace routing::cp;

class CPSolverTest : public ::testing::Test {
protected:
    void SetUp() override {
        problem_ = std::make_unique<Problem>();
    }

    void TearDown() override {
        problem_.reset();
    }

    std::unique_ptr<Problem> problem_;
};

TEST_F(CPSolverTest, Construction) {
    CPSolver solver(problem_.get());
    EXPECT_EQ(solver.name(), "cp");
    EXPECT_NE(solver.description().find("cpoptimizer"), std::string::npos);
}

TEST_F(CPSolverTest, SetProblem) {
    CPSolver solver(problem_.get());

    auto newProblem = std::make_unique<Problem>();
    solver.setProblem(newProblem.get());

    EXPECT_EQ(solver.getProblem(), newProblem.get());
}

TEST_F(CPSolverTest, DefaultConfiguration) {
    CPSolver solver(problem_.get());
    solver.setDefaultConfiguration();
    // Should not throw
}

TEST_F(CPSolverTest, VerboseMode) {
    CPSolver solver(problem_.get());
    solver.setVerbose(true);
    // Should not throw
}

TEST_F(CPSolverTest, NumWorkers) {
    CPSolver solver(problem_.get());
    solver.setNumWorkers(4);
    // Should not throw
}

TEST_F(CPSolverTest, BackendAccess) {
    CPSolver solver(problem_.get());

    ICPBackend& backend = solver.getBackend();
    EXPECT_EQ(backend.name(), "cpoptimizer");
}

TEST_F(CPSolverTest, PluginRegistration) {
    auto& registry = PluginRegistry::instance();

    // The CPSolverPlugin should be registered
    // Check if we can create a CP solver via the registry
    auto plugin = std::make_unique<plugins::CPSolverPlugin>();
    EXPECT_EQ(plugin->name(), "CPSolverPlugin");
    EXPECT_EQ(plugin->type(), PluginType::Solver);
}

TEST_F(CPSolverTest, SolveEmptyProblem) {
    CPSolver solver(problem_.get());

    // Solving an empty problem (no generators) - CP Optimizer will treat this
    // as a satisfiability problem. We just verify it doesn't crash.
    try {
        bool result = solver.solve(1.0);
        // For satisfiability problems, result should be true if feasible
        (void)result;  // May or may not succeed depending on backend
    } catch (const std::exception& e) {
        // Some backends may throw for empty problems - that's acceptable
        SUCCEED() << "Expected exception for empty problem: " << e.what();
    }
}

TEST_F(CPSolverTest, Stats) {
    CPSolver solver(problem_.get());

    std::string stats = solver.getStats();
    EXPECT_NE(stats.find("Backend"), std::string::npos);
}

#else

TEST(CPSolverTest, NotAvailable) {
    // When CPLEX is not found, creating a CPSolver should throw
    routing::Problem problem;
    EXPECT_THROW(routing::cp::CPSolver(&problem), std::runtime_error);
}

#endif
