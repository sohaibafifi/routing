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
#include "plugins/constraints/cp/CPCapacityGenerator.hpp"
#include "plugins/constraints/cp/CPRoutingGenerator.hpp"
#include "plugins/constraints/cp/CPTimeWindowGenerator.hpp"
#include "plugins/solvers/CPSolverPlugin/CPSolver.hpp"
#include "plugins/solvers/XCSP3SolverPlugin/XCSP3Solver.hpp"

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
        .def("solve_with_callback",
             [](ISolver& solver, double timeout, nb::callable callback) {
                 // Set callback that acquires GIL before calling Python
                 solver.setImprovementCallback([callback](Solution* sol, double cost) {
                     nb::gil_scoped_acquire acquire;
                     callback(sol, cost);
                 });

                 // Release GIL during solving for better parallelism
                 bool result;
                 {
                     nb::gil_scoped_release release;
                     result = solver.solve(timeout);
                 }

                 // Clear callback to avoid dangling reference
                 solver.setImprovementCallback(nullptr);

                 return result;
             },
             nb::arg("timeout") = 3600.0, nb::arg("callback") = nb::none(),
             R"doc(
                 Solve with a callback for solution improvements.

                 Args:
                     timeout: Time limit in seconds
                     callback: Function(solution, cost) called on each improvement

                 Example:
                     def on_improvement(sol, cost):
                         print(f"New best: {cost}")
                     solver.solve_with_callback(60, on_improvement)
             )doc")
        .def("set_improvement_callback",
             [](ISolver& solver, nb::callable callback) {
                 solver.setImprovementCallback([callback](Solution* sol, double cost) {
                     nb::gil_scoped_acquire acquire;
                     callback(sol, cost);
                 });
             },
             nb::arg("callback"),
             "Set a callback function(solution, cost) for solution improvements")
        .def("get_solution", &ISolver::getSolution, nb::rv_policy::reference,
             "Get the solution after solving")
        .def("get_objective_value", &ISolver::getObjectiveValue,
             "Get the objective value of the solution")
        .def("is_optimal", &ISolver::isOptimal,
             "Check if the solution is proven optimal")
        .def("get_stats", &ISolver::getStats,
             "Get solver statistics as string")
        .def("set_verbose", [](ISolver& solver, bool verbose) {
            if (auto* cpSolver = dynamic_cast<cp::CPSolver*>(&solver)) {
                cpSolver->setVerbose(verbose);
                return;
            }
            if (auto* xcspSolver = dynamic_cast<cp::XCSP3Solver*>(&solver)) {
                xcspSolver->setVerbose(verbose);
                return;
            }
            throw std::runtime_error("Verbose flag is not supported for solver: " + solver.name());
        }, nb::arg("verbose"), "Enable verbose logging (CP/XCSP3 only)")
        .def("add_cp_generators", [](ISolver& solver) {
            auto add_all = [](auto& target) {
                auto routing = std::make_unique<cp::generators::CPRoutingGenerator>();
                auto* routingPtr = routing.get();

                auto capacity = std::make_unique<cp::generators::CPCapacityGenerator>();
                capacity->setRoutingGenerator(routingPtr);

                auto timeWindow = std::make_unique<cp::generators::CPTimeWindowGenerator>();
                timeWindow->setRoutingGenerator(routingPtr);

                target.addGenerator(std::move(routing));
                target.addGenerator(std::move(capacity));
                target.addGenerator(std::move(timeWindow));
            };

            if (auto* cpSolver = dynamic_cast<cp::CPSolver*>(&solver)) {
                add_all(*cpSolver);
                return;
            }
            if (auto* xcspSolver = dynamic_cast<cp::XCSP3Solver*>(&solver)) {
                add_all(*xcspSolver);
                return;
            }
            throw std::runtime_error("CP generators are not supported for solver: " + solver.name());
        }, "Add default CP generators (routing, capacity, time windows)");

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
    m.def("solve", [](Problem* problem, const std::string& solver_type, double timeout, bool verbose) {
        ensure_plugins();
        auto& registry = PluginRegistry::instance();

        // Sync legacy pointers for backward compatibility with metaheuristic solvers
        problem->syncLegacyPointers();

        auto solver = registry.createSolver(solver_type, problem);
        if (!solver) {
            throw std::runtime_error("Unknown solver type: " + solver_type);
        }

        solver->setDefaultConfiguration();

        if (auto* cpSolver = dynamic_cast<cp::CPSolver*>(solver.get())) {
            cpSolver->setVerbose(verbose);
        } else if (auto* xcspSolver = dynamic_cast<cp::XCSP3Solver*>(solver.get())) {
            xcspSolver->setVerbose(verbose);
        }

        bool success = solver->solve(timeout);

        if (success) {
            Solution* sol = solver->getSolution();
            if (sol) {
                return sol->clone();
            }
        }

        return static_cast<Solution*>(nullptr);
    }, nb::rv_policy::take_ownership,
       nb::arg("problem"), nb::arg("solver_type") = "ga", nb::arg("timeout") = 60.0, nb::arg("verbose") = false,
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

    // Convenience solve with callback function
    m.def("solve_with_callback",
        [](Problem* problem, nb::callable callback,
           const std::string& solver_type, double timeout, bool verbose) {
            ensure_plugins();
            auto& registry = PluginRegistry::instance();

            // Sync legacy pointers for backward compatibility with metaheuristic solvers
            problem->syncLegacyPointers();

            auto solver = registry.createSolver(solver_type, problem);
            if (!solver) {
                throw std::runtime_error("Unknown solver type: " + solver_type);
            }

            solver->setDefaultConfiguration();

            if (auto* cpSolver = dynamic_cast<cp::CPSolver*>(solver.get())) {
                cpSolver->setVerbose(verbose);
            } else if (auto* xcspSolver = dynamic_cast<cp::XCSP3Solver*>(solver.get())) {
                xcspSolver->setVerbose(verbose);
            }

            // Set callback with GIL handling
            solver->setImprovementCallback([callback](Solution* sol, double cost) {
                nb::gil_scoped_acquire acquire;
                callback(sol, cost);
            });

            // Release GIL during solving
            bool success;
            {
                nb::gil_scoped_release release;
                success = solver->solve(timeout);
            }

            // Clear callback
            solver->setImprovementCallback(nullptr);

            if (success) {
                Solution* sol = solver->getSolution();
                if (sol) {
                    return sol->clone();
                }
            }

            return static_cast<Solution*>(nullptr);
        }, nb::rv_policy::take_ownership,
        nb::arg("problem"), nb::arg("callback"),
        nb::arg("solver_type") = "ga", nb::arg("timeout") = 60.0, nb::arg("verbose") = false,
        R"doc(
            Solve a problem with improvement callback.

            Args:
                problem: The problem to solve
                callback: Function(solution, cost) called on each improvement
                solver_type: Type of solver (default: "ga")
                timeout: Time limit in seconds (default: 60)
                verbose: Enable verbose logging (default: False)

            Returns:
                Solution if found, None otherwise

            Example:
                def on_improvement(sol, cost):
                    print(f"New best: {cost}")

                solution = routing.solve_with_callback(problem, on_improvement, "ga", 30)
        )doc");

    // Initialize plugins (called automatically, but can be called explicitly)
    m.def("init", []() {
        ensure_plugins();
    }, "Initialize the routing library (loads plugins)");
}
