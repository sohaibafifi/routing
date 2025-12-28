"""Tests for solution callbacks."""

import pytest
import os


DATA_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'data')


@pytest.fixture
def routing():
    """Import routing module."""
    import sys
    sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
    import routing
    return routing


@pytest.fixture
def test_problem(routing):
    """Load a test problem."""
    filepath = os.path.join(DATA_DIR, 'VRPTWTD/Solomon/Instances/25/r101.txt')
    if not os.path.exists(filepath):
        pytest.skip(f"Test file not found: {filepath}")
    return routing.load_solomon(filepath)


class TestSolveWithCallback:
    """Tests for solve_with_callback function."""

    def test_callback_called(self, routing, test_problem):
        """Test that callback is called during solving."""
        improvements = []

        def on_improvement(solution, cost):
            improvements.append(cost)

        solution = routing.solve_with_callback(
            test_problem, on_improvement, 'ga', timeout=3
        )

        assert solution is not None
        assert len(improvements) > 0, "Callback should be called at least once"

    def test_callback_costs_decrease(self, routing, test_problem):
        """Test that reported costs are decreasing."""
        costs = []

        def on_improvement(solution, cost):
            costs.append(cost)

        solution = routing.solve_with_callback(
            test_problem, on_improvement, 'ga', timeout=3
        )

        # Each cost should be less than the previous
        for i in range(1, len(costs)):
            assert costs[i] < costs[i-1] + 1e-6, \
                f"Costs should decrease: {costs[i-1]} -> {costs[i]}"

    def test_callback_receives_solution(self, routing, test_problem):
        """Test that callback receives valid solutions."""
        solutions_received = []

        def on_improvement(solution, cost):
            # Store solution info
            solutions_received.append({
                'cost': solution.cost if solution else None,
                'reported_cost': cost
            })

        solution = routing.solve_with_callback(
            test_problem, on_improvement, 'ga', timeout=3
        )

        assert len(solutions_received) > 0
        # Verify solution costs match reported costs (approximately)
        for entry in solutions_received:
            if entry['cost'] is not None:
                assert abs(entry['cost'] - entry['reported_cost']) < 1e-6


class TestSolverCallback:
    """Tests for solver.solve_with_callback method."""

    def test_solver_callback(self, routing, test_problem):
        """Test using callback through solver object."""
        solver = routing.create_solver('ga', test_problem)

        improvements = []

        def on_improvement(solution, cost):
            improvements.append(cost)

        solver.solve_with_callback(timeout=3, callback=on_improvement)

        assert len(improvements) > 0

    def test_set_improvement_callback(self, routing, test_problem):
        """Test setting callback before solving."""
        solver = routing.create_solver('ga', test_problem)

        improvements = []

        def on_improvement(solution, cost):
            improvements.append(cost)

        solver.set_improvement_callback(on_improvement)
        # Note: regular solve() won't trigger callbacks set via set_improvement_callback
        # because the callback is only used in solve_with_callback
