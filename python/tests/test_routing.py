"""
Tests for the routing Python bindings.

These tests require the native _routing_core module to be built first.
Run: cmake -DROUTING_BUILD_PYTHON=ON .. && make
Or:  pip install -e .
"""

import pytest

# Check if the native module is available
try:
    import routing._routing_core
    NATIVE_MODULE_AVAILABLE = True
except ImportError:
    NATIVE_MODULE_AVAILABLE = False

pytestmark = pytest.mark.skipif(
    not NATIVE_MODULE_AVAILABLE,
    reason="Native module _routing_core not built. Run: pip install -e ."
)


class TestImport:
    """Test that the module imports correctly."""

    def test_import_module(self):
        import routing
        assert hasattr(routing, "__version__")

    def test_import_classes(self):
        from routing import Problem, Solution, Tour, Client, Vehicle, Depot

    def test_import_functions(self):
        from routing import create_solver, solve, list_solvers

    def test_import_attributes(self):
        from routing import attributes
        assert hasattr(attributes, "AVAILABLE")


class TestProblem:
    """Test Problem class."""

    def test_create_empty_problem(self):
        from routing import Problem
        p = Problem()
        assert p.num_clients == 0
        assert p.num_vehicles == 0

    def test_add_client(self):
        from routing import Problem, set_client_location, set_client_demand
        p = Problem()
        c = p.add_client(1)
        assert c.get_id() == 1

        set_client_location(c, 10.0, 20.0)
        assert c.x == 10.0
        assert c.y == 20.0

        set_client_demand(c, 5)
        assert c.demand == 5

    def test_add_vehicle(self):
        from routing import Problem, set_vehicle_capacity
        p = Problem()
        v = p.add_vehicle(0)
        set_vehicle_capacity(v, 100)
        assert v.capacity == 100

    def test_add_depot(self):
        from routing import Problem, set_depot_location
        p = Problem()
        d = p.add_depot(0)
        set_depot_location(d, 0.0, 0.0)
        assert d.x == 0.0
        assert d.y == 0.0

    def test_get_clients(self):
        from routing import Problem
        p = Problem()
        p.add_client(1)
        p.add_client(2)
        p.add_client(3)
        assert p.num_clients == 3
        clients = p.get_clients()
        assert len(clients) == 3


class TestSolution:
    """Test Solution class."""

    def test_create_solution(self):
        from routing import Problem, create_solution
        p = Problem()
        s = create_solution(p)
        assert s.num_tours == 0
        assert s.cost == 0


class TestProblemBuilder:
    """Test ProblemBuilder helper."""

    def test_basic_problem(self):
        from routing.problem import ProblemBuilder

        problem = (ProblemBuilder()
            .with_depot(0, 0)
            .add_client(1, x=10, y=20, demand=5)
            .add_client(2, x=30, y=40, demand=3)
            .add_vehicle(capacity=100)
            .build())

        assert problem.num_clients == 2
        assert problem.num_vehicles == 1


class TestSolver:
    """Test Solver functions."""

    def test_list_solvers(self):
        from routing import list_solvers
        solvers = list_solvers()
        assert isinstance(solvers, list)
        # Should have at least some solvers registered
        assert len(solvers) >= 1


class TestAttributes:
    """Test attribute classes."""

    def test_geonode(self):
        import routing
        g = routing.attributes.GeoNode(10.5, 20.3)
        assert g.x == 10.5
        assert g.y == 20.3

    def test_consumer(self):
        import routing
        c = routing.attributes.Consumer(15)
        assert c.demand == 15

    def test_stock(self):
        import routing
        s = routing.attributes.Stock(100)
        assert s.capacity == 100

    def test_time_window(self):
        import routing
        tw = routing.attributes.TimeWindow(8.0, 12.0)
        assert tw.open == 8.0
        assert tw.close == 12.0

    def test_service_time(self):
        import routing
        st = routing.attributes.ServiceTime(0.5)
        assert st.service == 0.5


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
