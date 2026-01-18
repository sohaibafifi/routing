"""
Edge case tests for the routing Python bindings.

Tests empty problems, single client/vehicle, infeasible cases, etc.
"""

import pytest

# Check if the native module is available
try:
    import routing
    NATIVE_MODULE_AVAILABLE = True
except ImportError:
    NATIVE_MODULE_AVAILABLE = False

pytestmark = pytest.mark.skipif(
    not NATIVE_MODULE_AVAILABLE,
    reason="Native module _routing_core not built. Run: pip install -e ."
)


class TestEmptyProblems:
    """Test handling of empty or minimal problems."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_empty_problem(self):
        """Test creating an empty problem."""
        problem = routing.Problem()

        assert problem.num_clients == 0
        assert problem.num_vehicles == 0
        assert problem.num_depots == 0

    def test_problem_with_only_depot(self):
        """Test problem with only a depot."""
        problem = routing.Problem()
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        problem.enable_attributes(["GeoNode"])

        assert problem.num_depots == 1
        assert problem.num_clients == 0

    def test_problem_with_no_vehicles(self):
        """Test problem with clients but no vehicles."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 5)

        problem.enable_attributes(["GeoNode", "Consumer"])

        assert problem.num_clients == 1
        assert problem.num_vehicles == 0

        # Solving should handle this gracefully
        solution = routing.solve(problem, "ga", timeout=1.0)
        # Should return None or have unserved clients (no vehicles to serve with)
        if solution:
            assert len(solution.unserved) > 0, "Without vehicles, clients should be unserved"


class TestSingleElement:
    """Test problems with single client or vehicle."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_single_client(self):
        """Test problem with single client."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None
        if solution.is_feasible:
            assert solution.num_tours >= 1

    def test_single_vehicle(self):
        """Test problem with single vehicle and multiple clients."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        # Add 3 clients
        for i in range(1, 4):
            client = problem.add_client(i)
            client.add_attribute("GeoNode", i * 10, i * 10)
            client.add_attribute("Consumer", 5)

        # Single vehicle
        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None
        # Single vehicle should serve all if capacity allows
        assert solution.num_tours <= 1 or len(solution.unserved) > 0


class TestInfeasibleProblems:
    """Test infeasible problem scenarios."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_insufficient_capacity(self):
        """Test problem where total capacity < total demand."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        # Add clients with total demand = 30
        for i in range(1, 4):
            client = problem.add_client(i)
            client.add_attribute("GeoNode", i * 10, i * 10)
            client.add_attribute("Consumer", 10)

        # Add vehicle with insufficient capacity
        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 15)  # Only 15 < 30

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None
        # With insufficient capacity, some clients should be unserved
        assert len(solution.unserved) > 0, f"Expected unserved clients with capacity 15 < demand 30, but got {len(solution.unserved)} unserved"

    def test_zero_capacity_vehicle(self):
        """Test vehicle with zero capacity."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 0)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=1.0)

        # With zero capacity, client should be unserved
        if solution:
            assert len(solution.unserved) > 0, "With 0 capacity, client should be unserved"


class TestBoundaryValues:
    """Test boundary values and extreme cases."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_large_coordinates(self):
        """Test with large coordinate values."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10000, 10000)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None
        assert solution.cost > 0  # Should have non-zero distance

    def test_negative_coordinates(self):
        """Test with negative coordinates."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", -10, -10)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None

    def test_zero_demand_clients(self):
        """Test clients with zero demand."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        # Client with zero demand
        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 0)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        # Should not crash
        solution = routing.solve(problem, "ga", timeout=1.0)
        assert solution is not None


class TestAttributeEdgeCases:
    """Test edge cases for attributes."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_missing_attributes(self):
        """Test accessing attributes that haven't been set."""
        problem = routing.Problem()

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)

        problem.enable_attributes(["GeoNode"])

        # Accessing unset attributes should return None
        assert client.demand is None
        assert client.tw_open is None
        assert client.tw_close is None
        assert client.service_time is None
        assert client.pickup is None
        assert client.delivery is None
        assert client.profit is None

    def test_depot_missing_time_windows(self):
        """Test depot without time windows."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        problem.enable_attributes(["GeoNode"])

        # Should return None for missing attributes
        assert depot.tw_open is None
        assert depot.tw_close is None

    def test_overlapping_time_windows(self):
        """Test time windows where open > close."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)
        depot.add_attribute("Rendezvous", 100, 50)  # Invalid: open > close

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Rendezvous", 200, 100)  # Invalid

        problem.enable_attributes(["GeoNode", "Rendezvous"])

        # Should not crash, solver should handle
        solution = routing.solve(problem, "ga", timeout=1.0)
        # May be infeasible


class TestSolverTimeouts:
    """Test solver behavior with different timeouts."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def create_test_problem(self):
        """Create a standard test problem."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        for i in range(1, 6):
            client = problem.add_client(i)
            client.add_attribute("GeoNode", i * 10, i * 10)
            client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])
        return problem

    def test_very_short_timeout(self):
        """Test with very short timeout (0.1 seconds)."""
        problem = self.create_test_problem()

        solution = routing.solve(problem, "ga", timeout=0.1)

        # Should still return something even with short timeout
        assert solution is not None

    def test_zero_timeout(self):
        """Test with zero timeout."""
        problem = self.create_test_problem()

        # Should handle gracefully
        solution = routing.solve(problem, "ga", timeout=0.0)
        # May return None or quick solution


class TestListSolvers:
    """Test solver listing functionality."""

    def test_list_solvers(self):
        """Test that list_solvers returns expected solvers."""
        routing.init()

        solvers = routing.list_solvers()

        assert isinstance(solvers, list)
        assert len(solvers) > 0

        # Check for common solvers
        assert "ga" in solvers or "alns" in solvers


class TestSolutionCloning:
    """Test solution cloning and copying."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_solution_clone(self):
        """Test cloning a solution."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=2.0)

        if solution:
            cloned = solution.clone()

            assert cloned is not None
            assert cloned.cost == solution.cost
            assert cloned.num_tours == solution.num_tours


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
