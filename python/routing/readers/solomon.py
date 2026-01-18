"""
Solomon format reader for CVRPTW instances.

Solomon format structure:
  Line 1: Instance name
  Lines 2-4: (empty/header lines, skipped)
  Line 5: num_vehicles capacity
  Lines 6-9: (header lines, skipped)
  Line 10+: id x y demand ready_time due_date service_time

The first node (usually id=0) is the depot.
All subsequent nodes are clients.

Example file:
  C101

  VEHICLE
  NUMBER     CAPACITY
      25          200

  CUSTOMER
  CUST NO.  XCOORD.   YCOORD.    DEMAND   READY TIME  DUE DATE   SERVICE TIME
      0      40        50          0          0       1236          0
      1      45        68         10        912        967         90
      ...
"""

from pathlib import Path
from typing import Union

from .. import _routing_core as _core


def read_solomon(filepath: Union[str, Path]) -> _core.Problem:
    """
    Read a Solomon format CVRPTW instance file.

    Args:
        filepath: Path to the Solomon format .txt file

    Returns:
        Problem instance with depot, clients, vehicles, and attributes

    Raises:
        FileNotFoundError: If file doesn't exist
        ValueError: If file format is invalid

    Example:
        >>> problem = read_solomon("C101.txt")
        >>> print(f"Clients: {problem.num_clients}, Vehicles: {problem.num_vehicles}")
    """
    filepath = Path(filepath)
    if not filepath.exists():
        raise FileNotFoundError(f"File not found: {filepath}")

    problem = _core.Problem()

    with open(filepath, "r") as f:
        lines = f.readlines()

    if not lines:
        raise ValueError(f"Empty file: {filepath}")

    # Line 1: Instance name
    name = lines[0].strip()
    # problem.set_name(name)  # If available

    # Lines 2-4: Skip (empty or header)
    # Line 5: num_vehicles capacity
    vehicle_line = _find_vehicle_line(lines)
    if vehicle_line is None:
        raise ValueError("Could not find vehicle/capacity line")

    parts = vehicle_line.split()
    if len(parts) < 2:
        raise ValueError(f"Invalid vehicle line: {vehicle_line}")

    num_vehicles = int(parts[0])
    capacity = int(parts[1])

    # Add vehicles
    for k in range(num_vehicles):
        vehicle = problem.add_vehicle(k)
        vehicle.add_attribute("Stock", capacity)

    # Find and parse node data
    node_lines = _find_node_lines(lines)
    if not node_lines:
        raise ValueError("No node data found in file")

    first_node = True
    for line in node_lines:
        line = line.strip()
        if not line:
            continue

        parts = line.split()
        if len(parts) < 7:
            continue

        try:
            node_id = int(parts[0])
            x = float(parts[1])
            y = float(parts[2])
            demand = int(parts[3])
            ready_time = float(parts[4])
            due_date = float(parts[5])
            service_time = float(parts[6])
        except (ValueError, IndexError):
            continue

        if first_node:
            # First node is depot
            depot = problem.add_depot(node_id)
            depot.add_attribute("GeoNode", x, y)
            depot.add_attribute("Rendezvous", ready_time, due_date)
            first_node = False
        else:
            # Subsequent nodes are clients
            client = problem.add_client(node_id)
            client.add_attribute("GeoNode", x, y)
            client.add_attribute("Consumer", demand)
            client.add_attribute("Rendezvous", ready_time, due_date)
            client.add_attribute("ServiceQuery", service_time)

    # Attributes are automatically enabled when added
    return problem


def _find_vehicle_line(lines: list) -> str:
    """Find the line containing vehicle count and capacity."""
    for i, line in enumerate(lines):
        stripped = line.strip()
        # Look for a line with exactly 2 numbers (num_vehicles, capacity)
        # Usually around line 4-5
        if i >= 3 and i <= 6:
            parts = stripped.split()
            if len(parts) == 2:
                try:
                    int(parts[0])
                    int(parts[1])
                    return stripped
                except ValueError:
                    continue
    return None


def _find_node_lines(lines: list) -> list:
    """Find all lines containing node data (id x y demand ready due service)."""
    result = []
    in_data_section = False

    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue

        # Check if this looks like a data line (starts with a number)
        parts = stripped.split()
        if len(parts) >= 7:
            try:
                int(parts[0])  # Node ID
                float(parts[1])  # x
                float(parts[2])  # y
                int(parts[3])  # demand
                float(parts[4])  # ready
                float(parts[5])  # due
                float(parts[6])  # service
                result.append(stripped)
                in_data_section = True
            except ValueError:
                if in_data_section:
                    break  # End of data section
                continue

    return result
