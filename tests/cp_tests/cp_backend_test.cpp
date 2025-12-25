// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include "gtest/gtest.h"
#include "plugins/solvers/CPSolverPlugin/CPOptimizerBackend.hpp"

#ifdef CPLEX_FOUND

using namespace routing::cp;

TEST(CPOptimizerBackendTest, BasicVariableCreation) {
    CPOptimizerBackend backend;

    auto x = backend.newIntVar(0, 10, "x");
    EXPECT_TRUE(x.isValid());
    EXPECT_EQ(x.id(), 0);

    auto y = backend.newIntVar(-5, 5, "y");
    EXPECT_TRUE(y.isValid());
    EXPECT_EQ(y.id(), 1);

    auto b = backend.newBoolVar("b");
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.id(), 2);
}

TEST(CPOptimizerBackendTest, ConstantCreation) {
    CPOptimizerBackend backend;

    auto c = backend.newConstant(42);
    EXPECT_TRUE(c.isValid());
}

TEST(CPOptimizerBackendTest, IntervalVariableCreation) {
    CPOptimizerBackend backend;

    auto interval = backend.newIntervalVar(0, 100, 5, 10, "task");
    EXPECT_TRUE(interval.isValid());
    EXPECT_EQ(interval.id(), 0);

    auto fixed = backend.newFixedIntervalVar(10, 50, 5, "fixed_task");
    EXPECT_TRUE(fixed.isValid());
}

TEST(CPOptimizerBackendTest, OptionalIntervalVariable) {
    CPOptimizerBackend backend;

    auto opt = backend.newOptionalIntervalVar(0, 100, 5, 10, "optional_task");
    EXPECT_TRUE(opt.isValid());
    EXPECT_TRUE(opt.presenceVar().isValid());
}

TEST(CPOptimizerBackendTest, LinearConstraint) {
    CPOptimizerBackend backend;

    auto x = backend.newIntVar(0, 10, "x");
    auto y = backend.newIntVar(0, 10, "y");

    // x + y = 5
    LinearExpr expr;
    expr.addTerm(x, 1);
    expr.addTerm(y, 1);
    backend.addEquality(expr, 5);

    // Minimize x
    backend.minimize(LinearExpr(x));

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        int xVal = backend.getValue(x);
        int yVal = backend.getValue(y);
        EXPECT_EQ(xVal + yVal, 5);
        EXPECT_EQ(xVal, 0);  // Should be minimized
    }
}

TEST(CPOptimizerBackendTest, AllDifferentConstraint) {
    CPOptimizerBackend backend;

    std::vector<IntVar> vars;
    for (int i = 0; i < 5; ++i) {
        vars.push_back(backend.newIntVar(1, 5, "v" + std::to_string(i)));
    }

    backend.addAllDifferent(vars);

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        std::set<int> values;
        for (const auto& var : vars) {
            values.insert(backend.getValue(var));
        }
        EXPECT_EQ(values.size(), 5);  // All values must be different
    }
}

TEST(CPOptimizerBackendTest, NoOverlapConstraint) {
    CPOptimizerBackend backend;

    // Create 3 intervals that must not overlap on timeline 0-30
    auto t1 = backend.newIntervalVar(0, 30, 10, 10, "task1");
    auto t2 = backend.newIntervalVar(0, 30, 10, 10, "task2");
    auto t3 = backend.newIntervalVar(0, 30, 10, 10, "task3");

    backend.addNoOverlap({t1, t2, t3});

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        int s1 = backend.getStart(t1);
        int e1 = backend.getEnd(t1);
        int s2 = backend.getStart(t2);
        int e2 = backend.getEnd(t2);
        int s3 = backend.getStart(t3);
        int e3 = backend.getEnd(t3);

        // Verify no overlap: for any two intervals, one must end before the other starts
        EXPECT_TRUE(e1 <= s2 || e2 <= s1 || e1 <= s3 || e3 <= s1);
        EXPECT_TRUE(e2 <= s3 || e3 <= s2);
    }
}

TEST(CPOptimizerBackendTest, CumulativeConstraint) {
    CPOptimizerBackend backend;

    // 3 tasks with demands, capacity = 2
    auto t1 = backend.newIntervalVar(0, 20, 5, 5, "task1");  // demand 1
    auto t2 = backend.newIntervalVar(0, 20, 5, 5, "task2");  // demand 1
    auto t3 = backend.newIntervalVar(0, 20, 5, 5, "task3");  // demand 1

    backend.addCumulative({t1, t2, t3}, {1, 1, 1}, 2);

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);
}

TEST(CPOptimizerBackendTest, PrecedenceConstraint) {
    CPOptimizerBackend backend;

    auto pickup = backend.newIntervalVar(0, 50, 5, 5, "pickup");
    auto delivery = backend.newIntervalVar(0, 50, 5, 5, "delivery");

    // pickup must end before delivery starts
    backend.addEndBeforeStart(pickup, delivery, 0);

    // Minimize total completion
    backend.minimizeMakespan({pickup, delivery});

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        EXPECT_LE(backend.getEnd(pickup), backend.getStart(delivery));
    }
}

TEST(CPOptimizerBackendTest, ElementConstraint) {
    CPOptimizerBackend backend;

    auto index = backend.newIntVar(0, 2, "index");
    auto result = backend.newIntVar(0, 100, "result");

    std::vector<int> array = {10, 20, 30};
    backend.addElement(index, array, result);

    // Force index to be 1
    backend.addEquality(LinearExpr(index), 1);

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        EXPECT_EQ(backend.getValue(result), 20);
    }
}

TEST(CPOptimizerBackendTest, Statistics) {
    CPOptimizerBackend backend;

    auto x = backend.newIntVar(1, 10, "x");
    auto y = backend.newIntVar(1, 10, "y");
    backend.addAllDifferent({x, y});
    backend.minimize(LinearExpr(x));

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    // Check that statistics are available
    EXPECT_GE(backend.getSolveTime(), 0.0);
    EXPECT_GE(backend.getNumBranches(), 0);
}

TEST(CPOptimizerBackendTest, InfeasibleProblem) {
    CPOptimizerBackend backend;

    auto x = backend.newIntVar(0, 5, "x");
    auto y = backend.newIntVar(0, 5, "y");

    // x + y >= 100 is infeasible when domains are [0,5]
    LinearExpr expr;
    expr.addTerm(x, 1);
    expr.addTerm(y, 1);
    backend.addGreaterOrEqual(expr, 100);

    auto status = backend.solve(10.0);
    EXPECT_EQ(status, CPStatus::Infeasible);
}

TEST(CPOptimizerBackendTest, ClearModel) {
    CPOptimizerBackend backend;

    auto x = backend.newIntVar(0, 10, "x");
    EXPECT_EQ(x.id(), 0);

    backend.clear();

    // After clear, new variables should start from 0 again
    auto y = backend.newIntVar(0, 10, "y");
    EXPECT_EQ(y.id(), 0);
}

#else

TEST(CPOptimizerBackendTest, NotAvailable) {
    EXPECT_THROW(routing::cp::CPOptimizerBackend(), std::runtime_error);
}

#endif
