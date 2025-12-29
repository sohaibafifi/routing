// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include "gtest/gtest.h"
#include "plugins/solvers/CPSolverPlugin/ORToolsCPSATBackend.hpp"

#ifdef ORTOOLS_FOUND

using namespace routing::cp;

TEST(ORToolsCPSATBackendTest, BasicVariableCreation) {
    ORToolsCPSATBackend backend;

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

TEST(ORToolsCPSATBackendTest, ConstantCreation) {
    ORToolsCPSATBackend backend;

    auto c = backend.newConstant(42);
    EXPECT_TRUE(c.isValid());
}

TEST(ORToolsCPSATBackendTest, IntervalVariableCreation) {
    ORToolsCPSATBackend backend;

    auto interval = backend.newIntervalVar(0, 100, 5, 10, "task");
    EXPECT_TRUE(interval.isValid());
    EXPECT_EQ(interval.id(), 0);

    auto fixed = backend.newFixedIntervalVar(10, 50, 5, "fixed_task");
    EXPECT_TRUE(fixed.isValid());
}

TEST(ORToolsCPSATBackendTest, OptionalIntervalVariable) {
    ORToolsCPSATBackend backend;

    auto opt = backend.newOptionalIntervalVar(0, 100, 5, 10, "optional_task");
    EXPECT_TRUE(opt.isValid());
    EXPECT_TRUE(opt.presenceVar().isValid());
}

TEST(ORToolsCPSATBackendTest, LinearConstraint) {
    ORToolsCPSATBackend backend;

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

TEST(ORToolsCPSATBackendTest, AllDifferentConstraint) {
    ORToolsCPSATBackend backend;

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

TEST(ORToolsCPSATBackendTest, ElementConstraint) {
    ORToolsCPSATBackend backend;

    std::vector<int> array = {10, 20, 30, 40, 50};
    auto index = backend.newIntVar(0, 4, "index");
    auto result = backend.newIntVar(0, 100, "result");

    backend.addElement(index, array, result);

    // result == 30 => index == 2
    backend.addEquality(LinearExpr(result), 30);

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        EXPECT_EQ(backend.getValue(index), 2);
        EXPECT_EQ(backend.getValue(result), 30);
    }
}

TEST(ORToolsCPSATBackendTest, CircuitConstraint) {
    ORToolsCPSATBackend backend;

    int n = 5;
    std::vector<IntVar> next;
    for (int i = 0; i < n; ++i) {
        next.push_back(backend.newIntVar(0, n - 1, "next_" + std::to_string(i)));
    }

    backend.addCircuit(next);

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        // Verify it's a valid Hamiltonian circuit
        std::vector<bool> visited(n, false);
        int current = 0;
        for (int step = 0; step < n; ++step) {
            visited[current] = true;
            current = backend.getValue(next[current]);
        }
        // Should visit all nodes and return to start
        EXPECT_EQ(current, 0);
        for (bool v : visited) {
            EXPECT_TRUE(v);
        }
    }
}

TEST(ORToolsCPSATBackendTest, NoOverlapConstraint) {
    ORToolsCPSATBackend backend;

    auto task1 = backend.newFixedIntervalVar(0, 20, 5, "task1");
    auto task2 = backend.newFixedIntervalVar(0, 20, 5, "task2");

    backend.addNoOverlap({task1, task2});

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        int start1 = backend.getStart(task1);
        int end1 = backend.getEnd(task1);
        int start2 = backend.getStart(task2);
        int end2 = backend.getEnd(task2);

        // Tasks must not overlap
        EXPECT_TRUE(end1 <= start2 || end2 <= start1);
    }
}

TEST(ORToolsCPSATBackendTest, CumulativeConstraint) {
    ORToolsCPSATBackend backend;

    auto task1 = backend.newFixedIntervalVar(0, 10, 3, "task1");
    auto task2 = backend.newFixedIntervalVar(0, 10, 3, "task2");

    std::vector<int> demands = {5, 5};
    backend.addCumulative({task1, task2}, demands, 8);  // capacity 8, two tasks with demand 5 each

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);
}

TEST(ORToolsCPSATBackendTest, ObjectiveMinimization) {
    ORToolsCPSATBackend backend;

    auto x = backend.newIntVar(0, 10, "x");
    auto y = backend.newIntVar(0, 10, "y");

    backend.addEquality(x + y, 10);  // x + y = 10
    backend.minimize(LinearExpr(x));

    auto status = backend.solve(10.0);
    EXPECT_TRUE(status == CPStatus::Optimal || status == CPStatus::Feasible);

    if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
        EXPECT_EQ(backend.getValue(x), 0);  // x minimized
        EXPECT_EQ(backend.getValue(y), 10);  // y = 10 - x
    }
}

TEST(ORToolsCPSATBackendTest, SolverStatistics) {
    ORToolsCPSATBackend backend;

    auto x = backend.newIntVar(0, 10, "x");
    backend.minimize(LinearExpr(x));

    backend.solve(10.0);

    // Check that statistics are available
    EXPECT_GE(backend.getSolveTime(), 0.0);
    EXPECT_GE(backend.getNumBranches(), 0);
    EXPECT_GE(backend.getNumFailures(), 0);
}

#else

TEST(ORToolsCPSATBackendTest, NotAvailable) {
    GTEST_SKIP() << "OR-Tools not available";
}

#endif  // ORTOOLS_FOUND
