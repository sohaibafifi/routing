#include "gtest/gtest.h"
#include "plugins/solvers/XCSP3SolverPlugin/XCSP3Backend.hpp"
#include "plugins/solvers/XCSP3SolverPlugin/XCSP3Solver.hpp"
#include "core/Problem.hpp"
#include <fstream>
#include <filesystem>

// Simple test without complex problem setup
TEST(XCSP3BackendTest, BasicVariableCreation) {
    routing::cp::XCSP3Backend backend;
    backend.clear();

    auto var = backend.newIntVar(0, 10, "x");
    EXPECT_EQ(var.id(), 0);
    
    // Export and check if file is created
    EXPECT_TRUE(backend.exportModel("test_model.xml"));
    EXPECT_TRUE(std::filesystem::exists("test_model.xml"));
    std::filesystem::remove("test_model.xml");
}

TEST(XCSP3BackendTest, ConstraintGeneration) {
    routing::cp::XCSP3Backend backend;
    backend.clear();

    auto x = backend.newIntVar(0, 10, "x");
    auto y = backend.newIntVar(0, 10, "y");
    
    // x + y = 5
    routing::cp::LinearExpr expr;
    expr.addTerm(x, 1);
    expr.addTerm(y, 1);
    backend.addEquality(expr, 5);

    EXPECT_TRUE(backend.exportModel("test_constraints.xml"));
    
    // Verify content manually or just check existence
    std::ifstream in("test_constraints.xml");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("<intension> eq(add(mul(1,x),mul(1,y)),5) </intension>"), std::string::npos);
    
    in.close();
    std::filesystem::remove("test_constraints.xml");
}
