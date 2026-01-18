"""
Constants and enums for the routing library.

This module provides enums for attribute names and other constants
to improve type safety and IDE auto-completion.
"""

from enum import Enum


class Attribute(str, Enum):
    """
    Available attribute types for entities.

    Using string enum allows backward compatibility with string-based API
    while providing auto-completion and type safety.

    Example:
        >>> from routing.constants import Attribute
        >>> client.add_attribute(Attribute.GEONODE, 10, 20)
        >>> client.add_attribute(Attribute.CONSUMER, 15)
    """

    # Location and routing
    GEONODE = "GeoNode"
    """Coordinates (x, y) for distance calculation"""

    # Capacity
    CONSUMER = "Consumer"
    """Demand at a client node"""

    STOCK = "Stock"
    """Vehicle capacity"""

    # Time windows
    RENDEZVOUS = "Rendezvous"
    """Time window bounds (open, close)"""

    SERVICE_QUERY = "ServiceQuery"
    """Service time duration"""

    # Profit (Team Orienteering)
    PROFITER = "Profiter"
    """Profit value for orienteering problems"""

    # Pickup & Delivery
    PICKUP = "Pickup"
    """Pickup demand"""

    DELIVERY = "Delivery"
    """Delivery demand"""

    # Advanced constraints
    SOFT_TIME_WINDOWS = "SoftTimeWindows"
    """Soft time window penalties"""

    SYNCED = "Synced"
    """Temporal synchronization constraints"""


# Convenience aliases with PEP 8 naming
class Attr(str, Enum):
    """Short alias for Attribute enum (PEP 8 compliant names)."""

    GEO_NODE = "GeoNode"
    CONSUMER = "Consumer"
    STOCK = "Stock"
    RENDEZVOUS = "Rendezvous"
    SERVICE_QUERY = "ServiceQuery"
    PROFITER = "Profiter"
    PICKUP = "Pickup"
    DELIVERY = "Delivery"
    SOFT_TIME_WINDOWS = "SoftTimeWindows"
    SYNCED = "Synced"


# Constants for problem types
class ProblemType:
    """Common problem type attribute combinations."""

    TSP = [Attribute.GEONODE]
    """Traveling Salesman Problem"""

    VRP = [Attribute.GEONODE]
    """Vehicle Routing Problem"""

    CVRP = [Attribute.GEONODE, Attribute.CONSUMER, Attribute.STOCK]
    """Capacitated Vehicle Routing Problem"""

    VRPTW = [Attribute.GEONODE, Attribute.RENDEZVOUS, Attribute.SERVICE_QUERY]
    """Vehicle Routing Problem with Time Windows"""

    CVRPTW = [
        Attribute.GEONODE,
        Attribute.CONSUMER,
        Attribute.STOCK,
        Attribute.RENDEZVOUS,
        Attribute.SERVICE_QUERY
    ]
    """Capacitated VRP with Time Windows"""

    PDVRP = [
        Attribute.GEONODE,
        Attribute.PICKUP,
        Attribute.DELIVERY,
        Attribute.STOCK
    ]
    """Pickup and Delivery VRP"""

    TOP = [
        Attribute.GEONODE,
        Attribute.PROFITER,
        Attribute.RENDEZVOUS
    ]
    """Team Orienteering Problem"""


# All available attributes as a list
AVAILABLE_ATTRIBUTES = [attr.value for attr in Attribute]


__all__ = [
    'Attribute',
    'Attr',
    'ProblemType',
    'AVAILABLE_ATTRIBUTES',
]
