"""
High-level Problem API for the Routing Library.

This module provides a more Pythonic interface to the Problem class.
"""

from typing import Optional, List, Tuple
from . import _routing_core as _core


class ProblemBuilder:
    """
    Fluent builder for creating VRP problems.

    Example:
        problem = (ProblemBuilder()
            .with_depot(0, 0)
            .add_client(1, x=10, y=20, demand=5)
            .add_client(2, x=30, y=40, demand=3)
            .add_vehicle(capacity=100)
            .build())
    """

    def __init__(self):
        self._problem = _core.Problem()
        self._client_id = 1
        self._vehicle_id = 0
        self._depot_added = False

    def with_depot(
        self,
        x: float,
        y: float,
        tw_open: Optional[float] = None,
        tw_close: Optional[float] = None
    ) -> "ProblemBuilder":
        """Add depot with location and optional time window."""
        depot = self._problem.add_depot(0)
        _core.set_depot_location(depot, x, y)
        if tw_open is not None and tw_close is not None:
            _core.set_depot_time_window(depot, tw_open, tw_close)
        self._depot_added = True
        return self

    def add_client(
        self,
        client_id: Optional[int] = None,
        x: float = 0,
        y: float = 0,
        demand: int = 0,
        tw_open: Optional[float] = None,
        tw_close: Optional[float] = None,
        service_time: float = 0
    ) -> "ProblemBuilder":
        """Add a client with attributes."""
        if client_id is None:
            client_id = self._client_id
            self._client_id += 1

        client = self._problem.add_client(client_id)
        _core.set_client_location(client, x, y)

        if demand > 0:
            _core.set_client_demand(client, demand)

        if tw_open is not None and tw_close is not None:
            _core.set_client_time_window(client, tw_open, tw_close)

        if service_time > 0:
            _core.set_client_service_time(client, service_time)

        return self

    def add_clients_from_data(
        self,
        data: List[dict]
    ) -> "ProblemBuilder":
        """
        Add multiple clients from a list of dictionaries.

        Each dict should have keys: id (optional), x, y, demand, tw_open, tw_close, service_time
        """
        for item in data:
            self.add_client(
                client_id=item.get("id"),
                x=item.get("x", 0),
                y=item.get("y", 0),
                demand=item.get("demand", 0),
                tw_open=item.get("tw_open"),
                tw_close=item.get("tw_close"),
                service_time=item.get("service_time", 0)
            )
        return self

    def add_vehicle(
        self,
        capacity: int = 100,
        vehicle_id: Optional[int] = None
    ) -> "ProblemBuilder":
        """Add a vehicle with capacity."""
        if vehicle_id is None:
            vehicle_id = self._vehicle_id
            self._vehicle_id += 1

        vehicle = self._problem.add_vehicle(vehicle_id)
        _core.set_vehicle_capacity(vehicle, capacity)
        return self

    def add_vehicles(
        self,
        count: int,
        capacity: int = 100
    ) -> "ProblemBuilder":
        """Add multiple vehicles with same capacity."""
        for _ in range(count):
            self.add_vehicle(capacity)
        return self

    def build(self) -> _core.Problem:
        """Build and return the problem."""
        if not self._depot_added:
            # Add default depot at origin
            depot = self._problem.add_depot(0)
            _core.set_depot_location(depot, 0, 0)

        return self._problem


def load_solomon(filepath: str) -> _core.Problem:
    """
    Load a problem from Solomon format file.

    Args:
        filepath: Path to the .txt file

    Returns:
        Problem instance

    Note:
        This function will be implemented when readers are bound.
    """
    raise NotImplementedError("Solomon reader not yet bound to Python")


def load_tsplib(filepath: str) -> _core.Problem:
    """
    Load a problem from TSPLIB format file.

    Args:
        filepath: Path to the .vrp file

    Returns:
        Problem instance

    Note:
        This function will be implemented when readers are bound.
    """
    raise NotImplementedError("TSPLIB reader not yet bound to Python")
