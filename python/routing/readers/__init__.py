"""
File format readers for VRP instances.

Supports:
- Solomon format (CVRPTW)
- TSPLIB/CVRPLIB format (CVRP)
"""

from .solomon import read_solomon
from .tsplib import read_tsplib

__all__ = ["read_solomon", "read_tsplib"]
