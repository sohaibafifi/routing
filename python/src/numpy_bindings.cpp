// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"

namespace nb = nanobind;
using namespace routing;

void bind_numpy(nb::module_& m) {
    // Distance matrix
    m.def("set_distance_matrix",
        [](Problem& p, nb::ndarray<double, nb::ndim<2>, nb::c_contig, nb::device::cpu> matrix) {
            size_t n = matrix.shape(0);
            if (matrix.shape(1) != n) {
                throw std::runtime_error("Distance matrix must be square");
            }
            p.setDistanceMatrix(matrix.data(), n);
        },
        nb::arg("problem"), nb::arg("matrix"),
        R"doc(
        Set a custom distance matrix from a NumPy array.

        Args:
            problem: The Problem instance
            matrix: A square NxN numpy array of distances (float64)
                    Index 0 is depot, indices 1..N-1 are clients

        Example:
            >>> import numpy as np
            >>> distances = np.array([
            ...     [0, 10, 20],
            ...     [10, 0, 15],
            ...     [20, 15, 0]
            ... ], dtype=np.float64)
            >>> routing.set_distance_matrix(problem, distances)
        )doc");

    m.def("has_custom_distances",
        [](const Problem& p) { return p.hasCustomDistances(); },
        nb::arg("problem"),
        "Check if problem has a custom distance matrix set");

    m.def("clear_distance_matrix",
        [](Problem& p) { p.clearDistanceMatrix(); },
        nb::arg("problem"),
        "Clear the custom distance matrix");

    m.def("get_custom_distance",
        [](const Problem& p, size_t from, size_t to) {
            return p.getCustomDistance(from, to);
        },
        nb::arg("problem"), nb::arg("from_idx"), nb::arg("to_idx"),
        "Get distance between two nodes from custom matrix");

    // Bulk client data setters
    m.def("set_demands_bulk",
        [](Problem& p, nb::ndarray<int, nb::ndim<1>, nb::c_contig, nb::device::cpu> demands) {
            auto clients = p.getClients();
            size_t n = std::min(static_cast<size_t>(demands.shape(0)), clients.size());
            const int* data = demands.data();

            for (size_t i = 0; i < n; ++i) {
                auto* consumer = clients[i]->tryGetAttribute<attributes::Consumer>();
                if (consumer) {
                    // Consumer is immutable, need to replace
                    clients[i]->removeAttribute<attributes::Consumer>();
                }
                clients[i]->addAttribute<attributes::Consumer>(data[i]);
            }
        },
        nb::arg("problem"), nb::arg("demands"),
        R"doc(
        Set demands for all clients from a NumPy array.

        Args:
            problem: The Problem instance (must have clients added)
            demands: 1D array of integer demands, one per client

        Example:
            >>> demands = np.array([5, 10, 15, 20], dtype=np.int32)
            >>> routing.set_demands_bulk(problem, demands)
        )doc");

    m.def("set_locations_bulk",
        [](Problem& p,
           nb::ndarray<double, nb::ndim<2>, nb::c_contig, nb::device::cpu> coords) {
            if (coords.shape(1) != 2) {
                throw std::runtime_error("Coordinates array must have shape (N, 2)");
            }

            auto clients = p.getClients();
            size_t n = std::min(static_cast<size_t>(coords.shape(0)), clients.size());
            const double* data = coords.data();

            for (size_t i = 0; i < n; ++i) {
                double x = data[i * 2];
                double y = data[i * 2 + 1];

                auto* geo = clients[i]->tryGetAttribute<attributes::GeoNode>();
                if (geo) {
                    // GeoNode doesn't have setters, need to replace
                    clients[i]->removeAttribute<attributes::GeoNode>();
                    clients[i]->addAttribute<attributes::GeoNode>(x, y);
                } else {
                    clients[i]->addAttribute<attributes::GeoNode>(x, y);
                }
            }
        },
        nb::arg("problem"), nb::arg("coordinates"),
        R"doc(
        Set x,y coordinates for all clients from a NumPy array.

        Args:
            problem: The Problem instance (must have clients added)
            coordinates: 2D array of shape (N, 2) with x,y coordinates

        Example:
            >>> coords = np.array([[10, 20], [30, 40], [50, 60]], dtype=np.float64)
            >>> routing.set_locations_bulk(problem, coords)
        )doc");

    m.def("set_time_windows_bulk",
        [](Problem& p,
           nb::ndarray<double, nb::ndim<2>, nb::c_contig, nb::device::cpu> windows) {
            if (windows.shape(1) != 2) {
                throw std::runtime_error("Time windows array must have shape (N, 2)");
            }

            auto clients = p.getClients();
            size_t n = std::min(static_cast<size_t>(windows.shape(0)), clients.size());
            const double* data = windows.data();

            for (size_t i = 0; i < n; ++i) {
                double tw_open = data[i * 2];
                double tw_close = data[i * 2 + 1];

                auto* tw = clients[i]->tryGetAttribute<attributes::Rendezvous>();
                if (tw) {
                    clients[i]->removeAttribute<attributes::Rendezvous>();
                }
                clients[i]->addAttribute<attributes::Rendezvous>(tw_open, tw_close);
            }
        },
        nb::arg("problem"), nb::arg("windows"),
        R"doc(
        Set time windows for all clients from a NumPy array.

        Args:
            problem: The Problem instance (must have clients added)
            windows: 2D array of shape (N, 2) with [open, close] times

        Example:
            >>> windows = np.array([[0, 100], [10, 50], [20, 80]], dtype=np.float64)
            >>> routing.set_time_windows_bulk(problem, windows)
        )doc");

    m.def("set_service_times_bulk",
        [](Problem& p,
           nb::ndarray<double, nb::ndim<1>, nb::c_contig, nb::device::cpu> times) {
            auto clients = p.getClients();
            size_t n = std::min(static_cast<size_t>(times.shape(0)), clients.size());
            const double* data = times.data();

            for (size_t i = 0; i < n; ++i) {
                auto* svc = clients[i]->tryGetAttribute<attributes::ServiceQuery>();
                if (svc) {
                    clients[i]->removeAttribute<attributes::ServiceQuery>();
                }
                clients[i]->addAttribute<attributes::ServiceQuery>(data[i]);
            }
        },
        nb::arg("problem"), nb::arg("service_times"),
        R"doc(
        Set service times for all clients from a NumPy array.

        Args:
            problem: The Problem instance (must have clients added)
            service_times: 1D array of service times (float64)

        Example:
            >>> service = np.array([10, 15, 20, 5], dtype=np.float64)
            >>> routing.set_service_times_bulk(problem, service)
        )doc");

    m.def("set_vehicle_capacities_bulk",
        [](Problem& p,
           nb::ndarray<int, nb::ndim<1>, nb::c_contig, nb::device::cpu> capacities) {
            auto vehicles = p.getVehicles();
            size_t n = std::min(static_cast<size_t>(capacities.shape(0)), vehicles.size());
            const int* data = capacities.data();

            for (size_t i = 0; i < n; ++i) {
                auto* stock = vehicles[i]->tryGetAttribute<attributes::Stock>();
                if (stock) {
                    vehicles[i]->removeAttribute<attributes::Stock>();
                }
                vehicles[i]->addAttribute<attributes::Stock>(data[i]);
            }
        },
        nb::arg("problem"), nb::arg("capacities"),
        R"doc(
        Set capacities for all vehicles from a NumPy array.

        Args:
            problem: The Problem instance (must have vehicles added)
            capacities: 1D array of integer capacities

        Example:
            >>> caps = np.array([100, 100, 150], dtype=np.int32)
            >>> routing.set_vehicle_capacities_bulk(problem, caps)
        )doc");
}
