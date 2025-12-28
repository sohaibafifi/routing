// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include "gtest/gtest.h"
#include "plugins/solvers/MIPSolverPlugin/CPLEXMIPBackend.hpp"
#include "plugins/solvers/MIPSolverPlugin/HiGHSMIPBackend.hpp"

using namespace routing::mip;

// ========== HiGHS Backend Tests ==========

#ifdef HIGHS_FOUND

TEST(HiGHSMIPBackendTest, BasicVariableCreation) {
    HiGHSMIPBackend backend;
    EXPECT_EQ(backend.name(), "highs");

    auto x = backend.newVar(0.0, 10.0, "x");
    EXPECT_EQ(x.id(), 0);

    auto y = backend.newIntVar(0, 10, "y");
    EXPECT_EQ(y.id(), 1);

    auto b = backend.newBoolVar("b");
    EXPECT_EQ(b.id(), 2);

    EXPECT_EQ(backend.getNumVars(), 3);
}

TEST(HiGHSMIPBackendTest, SimpleLP) {
    HiGHSMIPBackend backend;

    // minimize x + y subject to x + y >= 10, x,y >= 0
    auto x = backend.newVar(0.0, 100.0, "x");
    auto y = backend.newVar(0.0, 100.0, "y");

    LinearExpr constraint;
    constraint.addTerm(x, 1.0);
    constraint.addTerm(y, 1.0);
    backend.addGreaterEqual(constraint, 10.0, "sum_constraint");

    LinearExpr objective;
    objective.addTerm(x, 1.0);
    objective.addTerm(y, 1.0);
    backend.minimize(objective);

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, MIPStatus::Optimal);

    EXPECT_NEAR(backend.getObjectiveValue(), 10.0, 0.01);
    EXPECT_NEAR(backend.getValue(x) + backend.getValue(y), 10.0, 0.01);
}

TEST(HiGHSMIPBackendTest, SimpleMIP) {
    HiGHSMIPBackend backend;

    // minimize 2x + 3y subject to x + y >= 5, x,y integer >= 0
    auto x = backend.newIntVar(0, 100, "x");
    auto y = backend.newIntVar(0, 100, "y");

    LinearExpr constraint;
    constraint.addTerm(x, 1.0);
    constraint.addTerm(y, 1.0);
    backend.addGreaterEqual(constraint, 5.0, "sum_constraint");

    LinearExpr objective;
    objective.addTerm(x, 2.0);
    objective.addTerm(y, 3.0);
    backend.minimize(objective);

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, MIPStatus::Optimal);

    int xVal = static_cast<int>(std::round(backend.getValue(x)));
    int yVal = static_cast<int>(std::round(backend.getValue(y)));
    EXPECT_GE(xVal + yVal, 5);
    EXPECT_EQ(2 * xVal + 3 * yVal, static_cast<int>(std::round(backend.getObjectiveValue())));
}

TEST(HiGHSMIPBackendTest, BinaryProblem) {
    HiGHSMIPBackend backend;

    // maximize 3x + 2y subject to x + y <= 1 (binary knapsack)
    auto x = backend.newBoolVar("x");
    auto y = backend.newBoolVar("y");

    LinearExpr constraint;
    constraint.addTerm(x, 1.0);
    constraint.addTerm(y, 1.0);
    backend.addLessEqual(constraint, 1.0, "capacity");

    LinearExpr objective;
    objective.addTerm(x, 3.0);
    objective.addTerm(y, 2.0);
    backend.maximize(objective);

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, MIPStatus::Optimal);

    int xVal = static_cast<int>(std::round(backend.getValue(x)));
    EXPECT_EQ(xVal, 1);  // Should pick x (higher profit)
    EXPECT_NEAR(backend.getObjectiveValue(), 3.0, 0.01);
}

TEST(HiGHSMIPBackendTest, EqualityConstraint) {
    HiGHSMIPBackend backend;

    auto x = backend.newVar(0.0, 10.0, "x");
    auto y = backend.newVar(0.0, 10.0, "y");

    // x + y = 6
    LinearExpr eq;
    eq.addTerm(x, 1.0);
    eq.addTerm(y, 1.0);
    backend.addEqual(eq, 6.0, "equality");

    // minimize x
    backend.minimize(LinearExpr(x));

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, MIPStatus::Optimal);

    EXPECT_NEAR(backend.getValue(x), 0.0, 0.01);
    EXPECT_NEAR(backend.getValue(y), 6.0, 0.01);
}

TEST(HiGHSMIPBackendTest, InfeasibleProblem) {
    HiGHSMIPBackend backend;

    auto x = backend.newVar(0.0, 5.0, "x");

    // x >= 100 is infeasible when x <= 5
    LinearExpr constraint;
    constraint.addTerm(x, 1.0);
    backend.addGreaterEqual(constraint, 100.0, "impossible");

    backend.minimize(LinearExpr(x));

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, MIPStatus::Infeasible);
}

TEST(HiGHSMIPBackendTest, ClearModel) {
    HiGHSMIPBackend backend;

    auto x = backend.newVar(0.0, 10.0, "x");
    EXPECT_EQ(x.id(), 0);

    backend.clear();

    // After clear, new variables should start from 0 again
    auto y = backend.newVar(0.0, 10.0, "y");
    EXPECT_EQ(y.id(), 0);
    EXPECT_EQ(backend.getNumVars(), 1);
}

TEST(HiGHSMIPBackendTest, Statistics) {
    HiGHSMIPBackend backend;

    auto x = backend.newVar(0.0, 10.0, "x");
    LinearExpr obj(x);
    backend.minimize(obj);

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, MIPStatus::Optimal);

    EXPECT_GE(backend.getSolveTime(), 0.0);
    EXPECT_GE(backend.getNumIterations(), 0);
}

#else

TEST(HiGHSMIPBackendTest, NotAvailable) {
    EXPECT_THROW(HiGHSMIPBackend(), std::runtime_error);
}

#endif

// ========== CPLEX Backend Tests ==========

#ifdef CPLEX_FOUND

TEST(CPLEXMIPBackendTest, BasicVariableCreation) {
    CPLEXMIPBackend backend;
    EXPECT_EQ(backend.name(), "cplex");

    auto x = backend.newVar(0.0, 10.0, "x");
    EXPECT_EQ(x.id(), 0);

    auto y = backend.newIntVar(0, 10, "y");
    EXPECT_EQ(y.id(), 1);

    auto b = backend.newBoolVar("b");
    EXPECT_EQ(b.id(), 2);
}

TEST(CPLEXMIPBackendTest, SimpleLP) {
    CPLEXMIPBackend backend;

    auto x = backend.newVar(0.0, 100.0, "x");
    auto y = backend.newVar(0.0, 100.0, "y");

    LinearExpr constraint;
    constraint.addTerm(x, 1.0);
    constraint.addTerm(y, 1.0);
    backend.addGreaterEqual(constraint, 10.0, "sum_constraint");

    LinearExpr objective;
    objective.addTerm(x, 1.0);
    objective.addTerm(y, 1.0);
    backend.minimize(objective);

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, MIPStatus::Optimal);

    EXPECT_NEAR(backend.getObjectiveValue(), 10.0, 0.01);
}

#else

TEST(CPLEXMIPBackendTest, NotAvailable) {
    EXPECT_THROW(CPLEXMIPBackend(), std::runtime_error);
}

#endif
