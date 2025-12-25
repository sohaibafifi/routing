// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/unique_ptr.h>

#include "core/interfaces/ISolver.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "plugins/PluginBundle.hpp"

namespace nb = nanobind;
using namespace routing;

// Ensure plugins are registered and initialized
static bool plugins_initialized = false;
void ensure_plugins() {
    if (!plugins_initialized) {
        plugins::registerCorePlugins();
        PluginRegistry::instance().initializeAll();
        plugins_initialized = true;
    }
}

void bind_solver(nb::module_& m) {
    // ISolver interface
    nb::class_<ISolver>(m, "SolverBase")
        .def("name", &ISolver::name, "Get solver name")
        .def("description", &ISolver::description, "Get solver description")
        .def("solve", &ISolver::solve, nb::arg("timeout") = 3600.0,
             "Solve the problem with optional timeout in seconds")
        .def("get_solution", &ISolver::getSolution, nb::rv_policy::reference,
             "Get the solution after solving")
        .def("get_objective_value", &ISolver::getObjectiveValue,
             "Get the objective value of the solution")
        .def("is_optimal", &ISolver::isOptimal,
             "Check if the solution is proven optimal")
        .def("get_stats", &ISolver::getStats,
             "Get solver statistics as string");

    // Solver factory function
    m.def("create_solver", [](const std::string& solver_type, Problem* problem) -> ISolver* {
        ensure_plugins();
        auto& registry = PluginRegistry::instance();

        // Sync legacy pointers for backward compatibility with metaheuristic solvers
        problem->syncLegacyPointers();

        auto solver = registry.createSolver(solver_type, problem);
        if (!solver) {
            throw std::runtime_error("Unknown solver type: " + solver_type);
        }

        return solver.release();
    }, nb::rv_policy::take_ownership,
       nb::arg("solver_type"), nb::arg("problem"),
       R"doc(
           Create a solver instance.

           Args:
               solver_type: Type of solver ("ga", "vns", "ma", "pso", "ls", "mip", "cp")
               problem: The problem to solve

           Returns:
               A solver instance

           Example:
               solver = routing.create_solver("ga", problem)
               solver.solve(timeout=60)
               solution = solver.get_solution()
       )doc");

    // List available solvers
    m.def("list_solvers", []() {
        ensure_plugins();
        return PluginRegistry::instance().availableSolvers();
    }, "List available solver types");

    // Convenience solve function
    m.def("solve", [](Problem* problem, const std::string& solver_type, double timeout) {
        ensure_plugins();
        auto& registry = PluginRegistry::instance();

        // Sync legacy pointers for backward compatibility with metaheuristic solvers
        problem->syncLegacyPointers();

        auto solver = registry.createSolver(solver_type, problem);
        if (!solver) {
            throw std::runtime_error("Unknown solver type: " + solver_type);
        }

        solver->setDefaultConfiguration();
        bool success = solver->solve(timeout);

        if (success) {
            Solution* sol = solver->getSolution();
            if (sol) {
                return sol->clone();
            }
        }

        return static_cast<Solution*>(nullptr);
    }, nb::rv_policy::take_ownership,
       nb::arg("problem"), nb::arg("solver_type") = "ga", nb::arg("timeout") = 60.0,
       R"doc(
           Solve a problem with the specified solver.

           Args:
               problem: The problem to solve
               solver_type: Type of solver (default: "ga")
               timeout: Time limit in seconds (default: 60)

           Returns:
               Solution if found, None otherwise

           Example:
               solution = routing.solve(problem, "ga", timeout=30)
               if solution:
                   print(f"Cost: {solution.cost}")
       )doc");

    // Initialize plugins (called automatically, but can be called explicitly)
    m.def("init", []() {
        ensure_plugins();
    }, "Initialize the routing library (loads plugins)");
}
