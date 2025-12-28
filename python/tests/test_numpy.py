"""Tests for NumPy integration."""

import pytest
import os
import numpy as np


@pytest.fixture
def routing():
    """Import routing module."""
    import sys
    sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
    import routing
    return routing


@pytest.fixture
def simple_problem(routing):
    """Create a simple test problem."""
    problem = routing.Problem()

    # Add depot
    depot = problem.add_depot(0)
    routing.set_depot_location(depot, 0, 0)

    # Add 5 clients
    for i in range(1, 6):
        c = problem.add_client(i)

    # Add vehicle
    v = problem.add_vehicle(0)

    return problem


class TestBulkSetters:
    """Tests for bulk data setters."""

    def test_set_locations_bulk(self, routing, simple_problem):
        """Test setting client locations in bulk."""
        coords = np.array([
            [10, 20],
            [30, 40],
            [50, 60],
            [70, 80],
            [90, 100]
        ], dtype=np.float64)

        routing.set_locations_bulk(simple_problem, coords)
        # If no exception, test passes

    def test_set_demands_bulk(self, routing, simple_problem):
        """Test setting client demands in bulk."""
        demands = np.array([5, 10, 15, 20, 25], dtype=np.int32)
        routing.set_demands_bulk(simple_problem, demands)

    def test_set_time_windows_bulk(self, routing, simple_problem):
        """Test setting client time windows in bulk."""
        windows = np.array([
            [0, 100],
            [10, 50],
            [20, 80],
            [30, 90],
            [40, 100]
        ], dtype=np.float64)

        routing.set_time_windows_bulk(simple_problem, windows)

    def test_set_service_times_bulk(self, routing, simple_problem):
        """Test setting client service times in bulk."""
        service_times = np.array([10, 15, 20, 25, 30], dtype=np.float64)
        routing.set_service_times_bulk(simple_problem, service_times)

    def test_set_vehicle_capacities_bulk(self, routing, simple_problem):
        """Test setting vehicle capacities in bulk."""
        caps = np.array([100], dtype=np.int32)
        routing.set_vehicle_capacities_bulk(simple_problem, caps)


class TestDistanceMatrix:
    """Tests for custom distance matrix."""

    def test_set_distance_matrix(self, routing, simple_problem):
        """Test setting a custom distance matrix."""
        n = 6  # depot + 5 clients
        dist_matrix = np.array([
            [0, 10, 20, 30, 40, 50],
            [10, 0, 15, 25, 35, 45],
            [20, 15, 0, 20, 30, 40],
            [30, 25, 20, 0, 15, 25],
            [40, 35, 30, 15, 0, 10],
            [50, 45, 40, 25, 10, 0]
        ], dtype=np.float64)

        routing.set_distance_matrix(simple_problem, dist_matrix)
        assert routing.has_custom_distances(simple_problem)

    def test_get_custom_distance(self, routing, simple_problem):
        """Test retrieving custom distances."""
        dist_matrix = np.array([
            [0, 10, 20, 30, 40, 50],
            [10, 0, 15, 25, 35, 45],
            [20, 15, 0, 20, 30, 40],
            [30, 25, 20, 0, 15, 25],
            [40, 35, 30, 15, 0, 10],
            [50, 45, 40, 25, 10, 0]
        ], dtype=np.float64)

        routing.set_distance_matrix(simple_problem, dist_matrix)

        assert routing.get_custom_distance(simple_problem, 0, 1) == 10.0
        assert routing.get_custom_distance(simple_problem, 1, 2) == 15.0
        assert routing.get_custom_distance(simple_problem, 4, 5) == 10.0

    def test_clear_distance_matrix(self, routing, simple_problem):
        """Test clearing custom distance matrix."""
        dist_matrix = np.zeros((6, 6), dtype=np.float64)
        routing.set_distance_matrix(simple_problem, dist_matrix)
        assert routing.has_custom_distances(simple_problem)

        routing.clear_distance_matrix(simple_problem)
        assert not routing.has_custom_distances(simple_problem)

    def test_non_square_matrix_raises(self, routing, simple_problem):
        """Test that non-square matrix raises an error."""
        non_square = np.array([[0, 1, 2], [1, 0, 1]], dtype=np.float64)

        with pytest.raises(RuntimeError):
            routing.set_distance_matrix(simple_problem, non_square)
