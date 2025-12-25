# PyCSP3 Routing Extension

## Overview

This project implements a **Python Composable API** for Vehicle Routing Problems (VRP), serving as a bridge between the [C++ Routing Library](../../README.md) concepts and the [XCSP3](http://www.xcsp.org/) Constraint Programming ecosystem.

The core idea is to provide a high-level, attribute-based modeling interface (similar to the upcoming C++ bindings detailed in [ROADMAP.md](../../ROADMAP.md)) that transparently generates native **PyCSP3** constraints. This allows users to define complex routing problems by simply "composing" attributes like Capacities, Time Windows, or Resources, without writing low-level CP variable/constraint logic manually.

## Key Features

-   **Composable Modeling**: Define problems by adding attributes (`Capacity`, `TimeWindow`) to entities (`Client`, `Vehicle`).
-   **Native PyCSP3 Generation**: Automatically translates high-level attributes into efficient PyCSP3 constraints (`Cumulative`, `Circuit`, `NoOverlap`, etc.).
-   **Solver Agnostic**: Generated models (in XCSP3 format) can be solved by any compliant solver, such as **ACE** (Abstract Constraint Engine) or **CoSoCo**.

## Implemented Plugins (Generators)

The library uses a plugin/generator architecture to handle specific problem aspects:

| Generator | Description | PyCSP3 Constraints |
| :--- | :--- | :--- |
| **Flow** | Handles basic routing logic (sequences, visits). | `Circuit` |
| **Capacity** | Manages demand and vehicle capacity. | `Cumulative` (or resource flow logic) |
| **TimeWindow** | Handles scheduling and time bounds. | `NoOverlap`, valid interval propagation |

## Usage

### Prerequisites

-   Python 3.10+
-   [PyCSP3](https://pycsp.org/) (`pip install pycsp3`)
-   [ACE Solver](https://github.com/xcsp3team/ace) (Java-based) or compliant XCSP3 solver.

### Example

```python
from routing import Problem, Solver
from routing.attributes import Capacity, TimeWindow

# 1. Define a VRP Problem
problem = Problem("cvrp_example")
problem.add_attribute("capacity")
problem.add_attribute("time_window")

# 2. Add Entities
depot = problem.add_depot(id=0, x=0, y=0)
client = problem.add_client(id=1, x=10, y=10)
vehicle = problem.add_vehicle(id=1)

# 3. Configure Attributes
depot.add_attribute("vehicle_capacity", Capacity(100))
client.add_attribute("demand", Capacity(10))
client.add_attribute("time_window", TimeWindow(0, 50, service_time=5))

# 4. Solve using ACE (via PyCSP3)
solver = Solver()
status = solver.solve(problem)
```

## Internal Documentation

-   [**Main README**](../../README.md): C++ Library compilation and options.
-   [**ROADMAP**](../../ROADMAP.md): Vision for the next-generation VRP solver, including Python bindings (Phase 3).
-   [**PROBLEMS**](../../PROBLEMS.md): Definitions of VRP variants supported by the core library.

## References & Ecosystem

-   **[XCSP3](http://www.xcsp.org/)**: An XML-based format for representing Constraint Satisfaction and Optimization Problems.
-   **[PyCSP3](https://pycsp.org/)**: A Python library for modeling combinatorial constrained problems.
-   **[ACE (Abstract Constraint Engine)](https://github.com/xcsp3team/ace)**: A generic Constraint Programming solver focused on XCSP3.
-   **[CoSoCo](https://github.com/xcsp3team/cosoco)**: A C++ CP solver for XCSP3.
