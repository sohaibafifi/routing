"""
TSPLIB/CVRPLIB format reader for CVRP instances.

TSPLIB format structure:
  NAME: instance_name
  TYPE: CVRP
  DIMENSION: n
  CAPACITY: c
  EDGE_WEIGHT_TYPE: EUC_2D
  NODE_COORD_SECTION
  1 x1 y1
  2 x2 y2
  ...
  DEMAND_SECTION
  1 d1
  2 d2
  ...
  DEPOT_SECTION
  1
  -1
  EOF

The number of vehicles can be:
- Specified as VEHICLES or NUM_VEHICLES
- Parsed from instance name (e.g., "A-n32-k5" has 5 vehicles)
- Defaulted to ceil(n/10) if not specified
"""

import math
import re
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

from .. import _routing_core as _core


def read_tsplib(filepath: Union[str, Path]) -> _core.Problem:
    """
    Read a TSPLIB/CVRPLIB format CVRP instance file.

    Args:
        filepath: Path to the .vrp file

    Returns:
        Problem instance with depot, clients, vehicles, and attributes

    Raises:
        FileNotFoundError: If file doesn't exist
        ValueError: If file format is invalid

    Example:
        >>> problem = read_tsplib("A-n32-k5.vrp")
        >>> print(f"Clients: {problem.num_clients}, Vehicles: {problem.num_vehicles}")
    """
    filepath = Path(filepath)
    if not filepath.exists():
        raise FileNotFoundError(f"File not found: {filepath}")

    problem = _core.Problem()

    with open(filepath, "r") as f:
        content = f.read()

    # Parse header key-value pairs
    name = _parse_value(content, "NAME")
    dimension = int(_parse_value(content, "DIMENSION", "0"))
    capacity = int(_parse_value(content, "CAPACITY", "0"))
    num_vehicles = int(_parse_value(content, "VEHICLES", "0") or
                       _parse_value(content, "NUM_VEHICLES", "0"))

    # Try to parse vehicles from name if not specified
    if num_vehicles == 0 and name:
        num_vehicles = _parse_vehicles_from_name(name)

    # Default vehicles if still not found
    if num_vehicles == 0:
        num_vehicles = max(1, math.ceil(dimension / 10.0)) if dimension > 0 else 1

    # Parse sections
    coords = _parse_coord_section(content, dimension)
    demands = _parse_demand_section(content, dimension)
    depot_ids = _parse_depot_section(content)

    # If no depot specified, use first node
    if not depot_ids and coords:
        depot_ids = [min(coords.keys())]

    # Add vehicles
    for k in range(num_vehicles):
        vehicle = problem.add_vehicle(k)
        vehicle.add_attribute("Stock", capacity)

    # Add nodes
    for node_id, (x, y) in sorted(coords.items()):
        if node_id in depot_ids:
            depot = problem.add_depot(node_id)
            depot.add_attribute("GeoNode", x, y)
        else:
            client = problem.add_client(node_id)
            client.add_attribute("GeoNode", x, y)
            demand = demands.get(node_id, 0)
            client.add_attribute("Consumer", demand)

    # Attributes are automatically enabled when added
    return problem


def _parse_value(content: str, key: str, default: str = "") -> str:
    """Parse a key: value line from content."""
    pattern = rf"^\s*{key}\s*:\s*(.+?)\s*$"
    match = re.search(pattern, content, re.MULTILINE | re.IGNORECASE)
    if match:
        return match.group(1).strip()
    return default


def _parse_vehicles_from_name(name: str) -> int:
    """
    Parse number of vehicles from instance name.
    E.g., "A-n32-k5" -> 5, "E-n101-k8" -> 8
    """
    match = re.search(r"[kK](\d+)", name)
    if match:
        return int(match.group(1))
    return 0


def _parse_coord_section(content: str, dimension: int) -> Dict[int, Tuple[float, float]]:
    """Parse NODE_COORD_SECTION."""
    coords = {}

    # Find section start
    match = re.search(r"NODE_COORD_SECTION\s*\n", content, re.IGNORECASE)
    if not match:
        return coords

    # Parse lines after section header
    start = match.end()
    lines = content[start:].split("\n")

    for line in lines:
        line = line.strip()
        if not line:
            continue
        if _is_section_header(line):
            break

        parts = line.split()
        if len(parts) >= 3:
            try:
                node_id = int(parts[0])
                x = float(parts[1])
                y = float(parts[2])
                coords[node_id] = (x, y)
            except ValueError:
                continue

    return coords


def _parse_demand_section(content: str, dimension: int) -> Dict[int, int]:
    """Parse DEMAND_SECTION."""
    demands = {}

    match = re.search(r"DEMAND_SECTION\s*\n", content, re.IGNORECASE)
    if not match:
        return demands

    start = match.end()
    lines = content[start:].split("\n")

    for line in lines:
        line = line.strip()
        if not line:
            continue
        if _is_section_header(line):
            break

        parts = line.split()
        if len(parts) >= 2:
            try:
                node_id = int(parts[0])
                demand = int(parts[1])
                demands[node_id] = demand
            except ValueError:
                continue

    return demands


def _parse_depot_section(content: str) -> List[int]:
    """Parse DEPOT_SECTION."""
    depot_ids = []

    match = re.search(r"DEPOT_SECTION\s*\n", content, re.IGNORECASE)
    if not match:
        return depot_ids

    start = match.end()
    lines = content[start:].split("\n")

    for line in lines:
        line = line.strip()
        if not line:
            continue
        if _is_section_header(line):
            break

        try:
            depot_id = int(line)
            if depot_id == -1:
                break
            depot_ids.append(depot_id)
        except ValueError:
            continue

    return depot_ids


def _is_section_header(line: str) -> bool:
    """Check if line is a section header or EOF."""
    line = line.strip().upper()
    return (
        line in ("EOF", "NODE_COORD_SECTION", "DEMAND_SECTION", "DEPOT_SECTION",
                 "EDGE_WEIGHT_SECTION", "DISPLAY_DATA_SECTION") or
        ":" in line  # Key: value line
    )
