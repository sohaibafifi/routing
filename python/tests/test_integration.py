"""
Integration tests for the routing Python bindings.

Tests actual solving with different solvers and problem types.
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


class TestBasicSolving:
    """Test actual problem solving with different solvers."""

    def setup_method(self):
        """Initialize routing library before each test."""
        routing.init()

    def create_simple_cvrp(self):
        """Create a simple CVRP instance for testing."""
        problem = routing.Problem()

        # Add depot at origin
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        # Add 5 clients
        clients_data = [
            (1, 10, 10, 5),
            (2, 20, 10, 10),
            (3, 10, 20, 8),
            (4, 20, 20, 12),
            (5, 15, 15, 7),
        ]

        for client_id, x, y, demand in clients_data:
            client = problem.add_client(client_id)
            client.add_attribute("GeoNode", x, y)
            client.add_attribute("Consumer", demand)

        # Add 2 vehicles with capacity 30
        for v_id in range(2):
            vehicle = problem.add_vehicle(v_id)
            vehicle.add_attribute("Stock", 30)

        # Enable attributes
        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        return problem

    def test_solve_with_ga(self):
        """Test solving with genetic algorithm."""
        problem = self.create_simple_cvrp()

        solution = routing.solve(problem, "ga", timeout=5.0)

        assert solution is not None
        assert solution.cost > 0
        assert solution.num_tours >= 1
        assert solution.is_feasible or len(solution.unserved) > 0

    def test_solve_with_vns(self):
        """Test solving with VNS."""
        problem = self.create_simple_cvrp()

        solution = routing.solve(problem, "vns", timeout=5.0)

        assert solution is not None
        assert solution.cost > 0

    def test_solve_with_alns(self):
        """Test solving with ALNS."""
        problem = self.create_simple_cvrp()

        solution = routing.solve(problem, "alns", timeout=5.0)

        assert solution is not None
        assert solution.cost > 0, f"ALNS returned solution with cost {solution.cost}, expected > 0"

    def test_solver_parameters(self):
        """Test setting solver parameters."""
        problem = self.create_simple_cvrp()

        solver = routing.create_solver("ga", problem)
        solver.set_param_int("iterMax", 1000)
        solver.set_param_bool("feasibleOnly", True)

        success = solver.solve(5.0)
        assert isinstance(success, bool)

    def test_solution_callback(self):
        """Test solving with improvement callback."""
        # Create a larger problem to ensure GA makes improvements
        problem = routing.Problem()
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        # Add 10 clients to ensure the GA will find improvements
        for i in range(1, 11):
            client = problem.add_client(i)
            client.add_attribute("GeoNode", i * 10, i * 15)
            client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        improvements = []

        def callback(solution, cost):
            improvements.append(cost)

        solution = routing.solve_with_callback(
            problem,
            callback,
            solver_type="ga",
            timeout=5.0
        )

        assert solution is not None
        # With a larger problem, GA should find at least one improvement
        assert len(improvements) >= 1, f"Expected at least 1 callback, got {len(improvements)}"


class TestProblemBuilder:
    """Test ProblemBuilder fluent API."""

    def test_builder_basic(self):
        """Test basic problem building."""
        from routing.problem import ProblemBuilder

        problem = (ProblemBuilder()
            .with_depot(0, 0)
            .add_client(x=10, y=20, demand=5)
            .add_client(x=30, y=40, demand=3)
            .add_vehicle(capacity=100)
            .build())

        assert problem.num_clients == 2
        assert problem.num_vehicles == 1
        assert problem.num_depots == 1

    def test_builder_with_time_windows(self):
        """Test building problem with time windows."""
        from routing.problem import ProblemBuilder

        problem = (ProblemBuilder()
            .with_depot(0, 0, tw_open=0, tw_close=1000)
            .add_client(x=10, y=20, demand=5, tw_open=50, tw_close=200)
            .add_client(x=30, y=40, demand=3, tw_open=100, tw_close=300)
            .add_vehicle(capacity=100)
            .build())

        assert problem.num_clients == 2
        clients = problem.get_clients()
        assert clients[0].tw_open == 50
        assert clients[0].tw_close == 200


class TestSolutionAPI:
    """Test Solution API extensions."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_solution_to_dict(self):
        """Test converting solution to dictionary."""
        problem = routing.Problem()
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=3.0)

        if solution:
            sol_dict = solution.to_dict()

            assert "cost" in sol_dict
            assert "num_tours" in sol_dict
            assert "is_feasible" in sol_dict
            assert "tours" in sol_dict
            assert "unserved" in sol_dict

    def test_solution_properties(self):
        """Test new solution properties."""
        problem = routing.Problem()
        solution = routing.create_solution(problem)

        # Test new properties
        assert hasattr(solution, "is_feasible")
        assert hasattr(solution, "total_distance")
        assert solution.total_distance == solution.cost


class TestTourAPI:
    """Test Tour API extensions."""

    def test_tour_to_list(self):
        """Test converting tour to list."""
        routing.init()

        problem = routing.Problem()
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        for i in range(1, 4):
            client = problem.add_client(i)
            client.add_attribute("GeoNode", i * 10, i * 10)
            client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        solution = routing.solve(problem, "ga", timeout=3.0)

        if solution and solution.num_tours > 0:
            tour = solution.get_tour(0)

            # Test to_list method
            client_ids = tour.to_list()
            assert isinstance(client_ids, list)

            # Test total_demand property
            assert hasattr(tour, "total_demand")
            assert hasattr(tour, "total_distance")


class TestNewAttributes:
    """Test new attribute bindings."""

    def test_pickup_delivery_attributes(self):
        """Test Pickup and Delivery attributes."""
        routing.init()

        problem = routing.Problem()
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        # Test pickup client
        pickup_client = problem.add_client(1)
        pickup_client.add_attribute("GeoNode", 10, 10)
        pickup_client.add_attribute("Pickup", 15)

        # Test delivery client
        delivery_client = problem.add_client(2)
        delivery_client.add_attribute("GeoNode", 20, 20)
        delivery_client.add_attribute("Delivery", 15)

        problem.enable_attributes(["GeoNode", "Pickup", "Delivery"])

        # Verify attributes
        assert pickup_client.pickup == 15
        assert delivery_client.delivery == 15

    def test_profit_attribute(self):
        """Test Profit attribute for TOP problems."""
        routing.init()

        problem = routing.Problem()
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Profiter", 50.5)

        problem.enable_attributes(["GeoNode", "Profiter"])

        assert client.profit == 50.5


class TestProblemAPI:
    """Test Problem API extensions."""

    def test_problem_properties(self):
        """Test new problem properties."""
        routing.init()

        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        for i in range(1, 6):
            client = problem.add_client(i)
            client.add_attribute("GeoNode", i * 10, i * 10)
            client.add_attribute("Consumer", i * 2)

        for v_id in range(3):
            vehicle = problem.add_vehicle(v_id)
            vehicle.add_attribute("Stock", 20)

        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        # Test new properties
        assert problem.num_depots == 1
        assert problem.total_demand == 2 + 4 + 6 + 8 + 10  # 30
        assert problem.total_capacity == 60  # 3 vehicles * 20

    def test_get_depot(self):
        """Test get_depot method."""
        routing.init()

        problem = routing.Problem()
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        problem.enable_attributes(["GeoNode"])

        # Test get_depot
        retrieved_depot = problem.get_depot(0)
        assert retrieved_depot is not None
        assert retrieved_depot.get_id() == 0


class TestGenericAttributeAPI:
    """Test the generic add_attribute API."""

    def test_add_attribute_generic(self):
        """Test generic add_attribute method."""
        routing.init()

        problem = routing.Problem()
        client = problem.add_client(1)

        # Test generic attribute addition
        client.add_attribute("GeoNode", 10.0, 20.0)
        client.add_attribute("Consumer", 15)

        problem.enable_attributes(["GeoNode", "Consumer"])

        assert client.x == 10.0
        assert client.y == 20.0
        assert client.demand == 15

    def test_has_attribute(self):
        """Test has_attribute method."""
        routing.init()

        problem = routing.Problem()
        client = problem.add_client(1)

        assert not client.has_attribute("GeoNode")

        client.add_attribute("GeoNode", 10.0, 20.0)

        assert client.has_attribute("GeoNode")
        assert not client.has_attribute("Consumer")


class TestAutomaticAttributeEnabling:
    """Test automatic attribute enabling feature."""

    def setup_method(self):
        """Initialize routing library."""
        routing.init()

    def test_auto_enable_on_add(self):
        """Test that attributes are automatically enabled when added."""
        problem = routing.Problem()

        # Add depot with GeoNode
        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        # GeoNode should now be enabled automatically
        # We can verify this by solving - if not enabled, solver would fail

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        # No explicit enable_attributes() call!
        # Solve should work because attributes were auto-enabled
        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None
        assert solution.cost > 0

    def test_auto_enable_multiple_attributes(self):
        """Test auto-enabling with multiple attribute types."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)
        depot.add_attribute("Rendezvous", 0, 1000)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 20)
        client.add_attribute("Consumer", 5)
        client.add_attribute("Rendezvous", 50, 200)
        client.add_attribute("ServiceQuery", 10.0)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        # All 5 attribute types should be auto-enabled
        solution = routing.solve(problem, "ga", timeout=3.0)

        assert solution is not None

    def test_explicit_enable_still_works(self):
        """Test that explicit enable_attributes() still works."""
        problem = routing.Problem()

        # Pre-enable attributes explicitly
        problem.enable_attributes(["GeoNode", "Consumer", "Stock"])

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        # Should work fine
        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None

    def test_mixed_explicit_and_auto_enable(self):
        """Test mixing explicit enable and auto-enable."""
        problem = routing.Problem()

        # Explicitly enable some attributes
        problem.enable_attributes(["GeoNode"])

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)

        client = problem.add_client(1)
        client.add_attribute("GeoNode", 10, 10)
        # This should auto-enable Consumer
        client.add_attribute("Consumer", 5)

        vehicle = problem.add_vehicle(0)
        # This should auto-enable Stock
        vehicle.add_attribute("Stock", 100)

        # Should work with both explicitly enabled and auto-enabled attributes
        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None

    def test_auto_enable_idempotent(self):
        """Test that auto-enabling the same attribute multiple times is safe."""
        problem = routing.Problem()

        depot = problem.add_depot(0)
        depot.add_attribute("GeoNode", 0, 0)  # First auto-enable

        # Adding GeoNode to other entities shouldn't cause issues
        client1 = problem.add_client(1)
        client1.add_attribute("GeoNode", 10, 10)  # Should not re-enable

        client2 = problem.add_client(2)
        client2.add_attribute("GeoNode", 20, 20)  # Should not re-enable

        vehicle = problem.add_vehicle(0)
        vehicle.add_attribute("Stock", 100)

        solution = routing.solve(problem, "ga", timeout=2.0)

        assert solution is not None


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
