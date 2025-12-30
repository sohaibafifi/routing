# Roadmap

This document outlines the development roadmap for the Routing library.

---

## Current Status

### Completed Features

::::{grid} 1 2 2 2
:gutter: 3

:::{grid-item-card} Composable Problem API
:class-card: sd-border-success

ECS-inspired attribute system for building any VRP variant at runtime without inheritance hierarchies.
:::

:::{grid-item-card} Multiple Solvers
:class-card: sd-border-success

GA, MA, VNS, PSO, Local Search metaheuristics with unified API.
:::

:::{grid-item-card} Exact Methods
:class-card: sd-border-success

MIP (CPLEX, HiGHS) and CP (CPLEX CP Optimizer, OR-Tools) solvers.
:::

:::{grid-item-card} Plugin Architecture
:class-card: sd-border-success

Extensible system for attributes, constraints, and solvers.
:::

:::{grid-item-card} Benchmark Readers
:class-card: sd-border-success

Solomon (CVRPTW) and TSPLIB (CVRP) format support.
:::

:::{grid-item-card} Python Bindings
:class-card: sd-border-success

Native Python API using nanobind with pip install support.
:::

::::

---

## In Development

| Feature | Status | Description |
|---------|--------|-------------|
| **[XCSP3](https://www.xcsp.org/) Export** | Active | Export problems to XCSP3 format for external CP solvers |
| **Incremental Evaluation** | Planned | O(1) move cost evaluation with delta caching |
| **Documentation** | Active | Comprehensive API docs and tutorials |

---

## Planned Features

### High Priority

| Feature | Description | Target |
|---------|-------------|--------|
| **Hybrid CP + LNS** | CP-based repair operator in Large Neighborhood Search | Q1 2026 |
| **ALNS Solver** | Adaptive Large Neighborhood Search with operator selection | Q1 2026 |
| **Parallel Solving** | Multi-threaded solver orchestration and portfolio | Q2 2026 |

### Medium Priority

| Feature | Description |
|---------|-------------|
| **JSON/YAML Format** | Universal instance format for all problem types |
| **Solution Visualization** | Plot routes and solution statistics |
| **Warm Starting** | Initialize solvers with existing solutions |
| **Callback System** | Progress callbacks for monitoring long solves |

### Future Considerations

| Feature | Description |
|---------|-------------|
| **GPU Acceleration** | CUDA-based distance matrix computation |
| **Cloud Deployment** | REST API and containerized solving |
| **Machine Learning** | ML-based operator selection and parameter tuning |

---

## Architecture Overview

```
                    ┌─────────────────────────────────────┐
                    │          Applications               │
                    │   (Python scripts, C++ examples)    │
                    └─────────────────────────────────────┘
                                      │
                    ┌─────────────────────────────────────┐
                    │          Python Bindings            │
                    │           (nanobind)                │
                    └─────────────────────────────────────┘
                                      │
     ┌────────────────────────────────┼────────────────────────────────┐
     │                                │                                │
┌────┴────┐                    ┌──────┴──────┐                   ┌─────┴─────┐
│ Problem │                    │   Solvers   │                   │  Plugins  │
│  Layer  │                    │    Layer    │                   │   Layer   │
├─────────┤                    ├─────────────┤                   ├───────────┤
│Attributes│                   │GA, MA, VNS  │                   │Attributes │
│Entities  │                   │PSO, LS      │                   │Constraints│
│Distances │                   │MIP, CP      │                   │Evaluators │
└─────────┘                    └─────────────┘                   └───────────┘
                                      │
                    ┌─────────────────────────────────────┐
                    │       Optimization Backends         │
                    │  CPLEX │ HiGHS │ OR-Tools │ Gurobi  │
                    └─────────────────────────────────────┘
```

---

## Version History

### v0.3.0 (Current)

- Composable attribute system
- OR-Tools CP-SAT backend
- HiGHS MIP backend
- Python bindings with pip install
- Documentation site

### v0.2.0

- Plugin architecture
- MIP solver (CPLEX)
- CP Optimizer backend
- Solomon and TSPLIB readers

### v0.1.0

- Initial release
- Genetic Algorithm solver
- Basic VRP/CVRP/CVRPTW support

---

## Contributing to the Roadmap

Have ideas for new features? We welcome contributions!

- **Feature Requests**: Open an issue on [GitHub](https://github.com/sohaibafifi/routing/issues)
- **Discussions**: Join the conversation on [GitHub Discussions](https://github.com/sohaibafifi/routing/discussions)
- **Pull Requests**: See [Contributing](contributing.md) for guidelines

---

## License

This project is available for research purposes at non-commercial and academic institutions.
