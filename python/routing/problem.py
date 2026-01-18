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
        self._needs_geo = False
        self._needs_consumer = False
        self._needs_stock = False
        self._needs_rendezvous = False
        self._needs_service = False

    def with_depot(
        self,
        x: float,
        y: float,
        tw_open: Optional[float] = None,
        tw_close: Optional[float] = None
    ) -> "ProblemBuilder":
        """Add depot with location and optional time window."""
        depot = self._problem.add_depot(0)
        depot.add_attribute("GeoNode", x, y)
        self._needs_geo = True
        if tw_open is not None and tw_close is not None:
            depot.add_attribute("Rendezvous", tw_open, tw_close)
            self._needs_rendezvous = True
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
        client.add_attribute("GeoNode", x, y)
        self._needs_geo = True

        if demand > 0:
            client.add_attribute("Consumer", demand)
            self._needs_consumer = True

        if tw_open is not None and tw_close is not None:
            client.add_attribute("Rendezvous", tw_open, tw_close)
            self._needs_rendezvous = True

        if service_time > 0 or (tw_open is not None and tw_close is not None):
            client.add_attribute("ServiceQuery", service_time)
            self._needs_service = True

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
        vehicle.add_attribute("Stock", capacity)
        self._needs_stock = True
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
            depot.add_attribute("GeoNode", 0, 0)
            self._needs_geo = True

        attributes = []
        if self._needs_geo:
            attributes.append("GeoNode")
        if self._needs_consumer:
            attributes.append("Consumer")
        if self._needs_stock:
            attributes.append("Stock")
        if self._needs_rendezvous:
            attributes.append("Rendezvous")
        if self._needs_service:
            attributes.append("ServiceQuery")

        if attributes:
            self._problem.enable_attributes(attributes)

        return self._problem


def load_solomon(filepath: str) -> _core.Problem:
    """
    Load a problem from Solomon format file.

    Args:
        filepath: Path to the .txt file (CVRPTW format)

    Returns:
        Problem instance with time windows and service times

    Example:
        >>> problem = load_solomon("C101.txt")
        >>> print(f"Clients: {problem.num_clients}")
    """
    from .readers.solomon import read_solomon
    return read_solomon(filepath)


def load_tsplib(filepath: str) -> _core.Problem:
    """
    Load a problem from TSPLIB/CVRPLIB format file.

    Args:
        filepath: Path to the .vrp file (CVRP format)

    Returns:
        Problem instance with demands

    Example:
        >>> problem = load_tsplib("A-n32-k5.vrp")
        >>> print(f"Clients: {problem.num_clients}")
    """
    from .readers.tsplib import read_tsplib
    return read_tsplib(filepath)
