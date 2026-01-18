// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/optional.h>

namespace nb = nanobind;

// Forward declarations for submodule binding functions
void bind_problem(nb::module_& m);
void bind_solution(nb::module_& m);
void bind_solver(nb::module_& m);
void bind_attributes(nb::module_& m);
void bind_numpy(nb::module_& m);

NB_MODULE(_routing_core, m) {
    m.doc() = R"doc(
        Routing Library - Core C++ Bindings

        A composable, multi-solver VRP framework.

        This module provides Python bindings to the C++ routing library,
        enabling high-performance vehicle routing problem solving.
    )doc";

    // Version info
    m.attr("__version__") = "0.1.1";

    // Bind all components
    bind_problem(m);
    bind_solution(m);
    bind_solver(m);
    bind_attributes(m);
    bind_numpy(m);
}
