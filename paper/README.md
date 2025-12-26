# Technical Paper: A Composable Attribute Framework for Vehicle Routing Problems

This directory contains the LaTeX source, Beamer presentation, and automated benchmark suite for the technical paper.

## Contents

```
paper/
├── routing_composable_framework.tex   # Main paper (19 pages)
├── presentation.tex                    # Beamer slides (21 slides)
├── Makefile                           # Build automation
├── README.md                          # This file
└── benchmarks/
    ├── run_benchmarks.py              # Python benchmark suite
    ├── results/                       # JSON results
    ├── tables/                        # Generated LaTeX tables
    └── plots/                         # Generated pgfplots data
```

## Quick Start

```bash
# Build paper and presentation
make all

# Or individually
make paper         # Build paper PDF
make presentation  # Build slides PDF
```

## Automated Test Protocol

The benchmark suite provides reproducible experiments for the paper using the Python API.

### Prerequisites

1. **Build the C++ project:**
```bash
make build-project
```

2. **Install Python bindings:**
```bash
make install-python
# Or manually: cd ../python && pip install -e .
```

### Running Benchmarks

```bash
# Quick test (~2 minutes, 2 instances)
make quick-bench

# Full benchmark suite (~2-4 hours)
make full-bench

# Single instance benchmark
make bench-instance INSTANCE=R101 SOLVER=ga TIMEOUT=60

# Regenerate tables from existing results
make tables
```

### Python API Usage

The benchmark script uses the Python API directly:

```python
import routing
from routing.problem import ProblemBuilder

# Create CVRPTW problem
problem = (ProblemBuilder()
    .with_depot(x=0, y=0, tw_open=0, tw_close=200)
    .add_client(1, x=20, y=20, demand=10, tw_open=0, tw_close=100, service_time=10)
    .add_client(2, x=30, y=40, demand=15, tw_open=10, tw_close=80, service_time=10)
    .add_vehicles(3, capacity=50)
    .build())

# Initialize and solve
routing.init()
solution = routing.solve(problem, "ga", timeout=30)

print(f"Cost: {solution.cost}")
for tour in solution.get_tours():
    print(f"  Route: {tour.get_client_ids()}")
```

### Available Solvers

| Solver | Name | Description |
|--------|------|-------------|
| `ga` | Genetic Algorithm | Population-based evolution |
| `vns` | Variable Neighborhood Search | Systematic neighborhood exploration |
| `ma` | Memetic Algorithm | Hybrid GA + local search |
| `pso` | Particle Swarm | Swarm intelligence |
| `ls` | Local Search | Neighborhood improvement |
| `mip` | Mixed Integer Programming | Exact (requires CPLEX) |
| `cp` | Constraint Programming | Exact (requires CPLEX) |

### Benchmark Configuration

Edit `benchmarks/run_benchmarks.py`:

```python
# Timeout settings
DEFAULT_TIMEOUT = 60   # seconds per instance
QUICK_TIMEOUT = 30     # for quick tests

# Instance sets
INSTANCES = {
    "solomon_r1": {
        "path": DATA_DIR / "CVRPTW",
        "instances": ["R101", "R102", "R103", "R104", "R105"],
    },
    # Add more instance sets...
}

# Solvers to benchmark
METAHEURISTIC_SOLVERS = ["ga", "vns", "ma", "pso", "ls"]
```

### Generated Outputs

| File | Description |
|------|-------------|
| `results/*.json` | Raw benchmark results |
| `tables/overhead.tex` | Compositional overhead table |
| `tables/solver_comparison.tex` | Solver performance comparison |
| `tables/loc_comparison.tex` | Lines of code reduction |
| `plots/solver_times.dat` | Data for pgfplots |

### C++ Benchmark Executable

For environments without Python, use the C++ benchmark CLI:

```bash
# Build
make build-project

# Run
./build/examples/benchmark/benchmark \
    --instance data/CVRPTW/R101.txt \
    --solver ga \
    --timeout 60

# Output (JSON)
{
  "instance": "R101",
  "solver": "ga",
  "variant": "cvrptw",
  "objective": 1650.80,
  "time_seconds": 58.234,
  "num_vehicles": 19,
  "num_clients": 100,
  "status": "success"
}
```

## Paper Structure

1. **Introduction** - VRP variants, software challenges, contributions
2. **Background** - Existing frameworks, ECS architecture
3. **System Architecture** - Plugin-based layered design
4. **Composable Attributes** - Core innovation (ECS-inspired)
5. **Constraint Generation** - Auto-activation mechanism
6. **Solver Framework** - MIP, CP, metaheuristics
7. **Python Bindings** - nanobind, PyCSP3
8. **Experiments** - Overhead, solver comparison
9. **Discussion** - Trade-offs, limitations
10. **Conclusion** - Summary, future work

## Presentation

21 slides with TikZ diagrams:

| Section | Slides | Content |
|---------|--------|---------|
| Introduction | 3 | VRP family, composable solution, contributions |
| Architecture | 2 | Layer diagram, plugin lifecycle |
| Attributes | 4 | ECS inspiration, attribute table, variants matrix |
| Constraints | 3 | Auto-activation, generator interface, formulations |
| Solvers | 2 | Multi-paradigm interface, XCSP3 integration |
| Python | 2 | nanobind API, PyCSP3 pipeline |
| Results | 3 | Overhead, LOC reduction, framework comparison |
| Conclusion | 2 | Summary, thank you |

## Full Workflow

```bash
# 1. Setup
make build-project
make install-python

# 2. Run experiments
make full-bench

# 3. Build deliverables
make all

# 4. (Optional) Clean
make clean
```

## arXiv Submission

```bash
# Verify compilation
make all

# Create archive
zip arxiv.zip \
    routing_composable_framework.tex \
    benchmarks/tables/*.tex

# Submit to https://arxiv.org/submit
```

**Categories:**
- **Primary:** cs.AI or cs.DS
- **Secondary:** cs.SE, math.OC

## Dependencies

### LaTeX
- `beamer` with `metropolis` theme
- `tikz`, `pgfplots`
- `booktabs`, `listings`
- `fontawesome5`, `natbib`

### Python
- Python 3.8+
- `routing` package (from project)

### C++ (optional)
- CMake 3.16+
- C++20 compiler
