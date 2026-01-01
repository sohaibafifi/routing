"""
High-level Solver API for the Routing Library.

This module provides a more Pythonic interface to the solvers.
"""

from typing import Optional, Callable, Any
from . import _routing_core as _core


class Solver:
    """
    High-level solver wrapper with convenient interface.

    Example:
        solver = Solver("ga", problem)
        solver.timeout = 60
        solution = solver.solve()

        # Or use the context manager
        with Solver("vns", problem) as solver:
            solution = solver.solve(timeout=30)
    """

    def __init__(
        self,
        solver_type: str = "ga",
        problem: Optional[_core.Problem] = None
    ):
        """
        Create a new solver.

        Args:
            solver_type: Type of solver ("alns", "ga", "vns", "ma", "pso", "ls", "mip", "cp")
            problem: Problem to solve (can be set later)
        """
        self.solver_type = solver_type
        self._problem = problem
        self._solver = None
        self._solution = None
        self.timeout = 60.0
        self._verbose = False

        if problem is not None:
            self._create_solver()

    def _create_solver(self):
        """Create the underlying solver instance."""
        if self._problem is None:
            raise ValueError("Problem must be set before creating solver")
        self._solver = _core.create_solver(self.solver_type, self._problem)
        try:
            self._solver.set_verbose(self._verbose)
        except AttributeError:
            pass

    @property
    def problem(self) -> Optional[_core.Problem]:
        """Get the problem being solved."""
        return self._problem

    @problem.setter
    def problem(self, value: _core.Problem):
        """Set the problem to solve."""
        self._problem = value
        self._solver = None  # Reset solver

    def solve(self, timeout: Optional[float] = None) -> Optional[_core.Solution]:
        """
        Solve the problem.

        Args:
            timeout: Time limit in seconds (uses self.timeout if not specified)

        Returns:
            Solution if found, None otherwise
        """
        if self._solver is None:
            self._create_solver()

        timeout = timeout or self.timeout
        success = self._solver.solve(timeout)

        if success:
            self._solution = self._solver.get_solution()
            return self._solution

        return None

    def add_cp_generators(self) -> None:
        """
        Add default CP generators (routing, capacity, time windows).

        Only applies to CP/XCSP3 solvers.
        """
        if self._solver is None:
            self._create_solver()
        self._solver.add_cp_generators()

    @property
    def solution(self) -> Optional[_core.Solution]:
        """Get the solution (after solving)."""
        return self._solution

    @property
    def objective_value(self) -> float:
        """Get objective value (after solving)."""
        if self._solver is None:
            return float("inf")
        return self._solver.get_objective_value()

    @property
    def is_optimal(self) -> bool:
        """Check if solution is proven optimal."""
        if self._solver is None:
            return False
        return self._solver.is_optimal()

    @property
    def stats(self) -> str:
        """Get solver statistics."""
        if self._solver is None:
            return ""
        return self._solver.get_stats()

    @property
    def verbose(self) -> bool:
        """Get verbose flag."""
        return self._verbose

    @verbose.setter
    def verbose(self, value: bool):
        """Set verbose flag (CP/XCSP3 only)."""
        self._verbose = bool(value)
        if self._solver is None:
            return
        try:
            self._solver.set_verbose(self._verbose)
        except AttributeError:
            pass

    def __enter__(self):
        """Context manager entry."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        pass  # No cleanup needed

    def __repr__(self):
        return f"<Solver type={self.solver_type} problem={self._problem}>"


def quick_solve(
    problem: _core.Problem,
    solver_type: str = "ga",
    timeout: float = 60.0
) -> Optional[_core.Solution]:
    """
    Quick solve a problem with default settings.

    Args:
        problem: Problem to solve
        solver_type: Type of solver
        timeout: Time limit in seconds

    Returns:
        Solution if found, None otherwise

    Example:
        solution = quick_solve(problem, "ga", timeout=30)
    """
    return _core.solve(problem, solver_type, timeout)


def available_solvers() -> list:
    """
    Get list of available solver types.

    Returns:
        List of solver type names
    """
    return _core.list_solvers()
