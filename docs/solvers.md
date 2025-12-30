# Solvers

The Routing library provides multiple solver backends for different needs.

---

## Overview

| Category | Solvers | Best For |
|----------|---------|----------|
| **Metaheuristics** | GA, MA, VNS, PSO, LS | Large instances, fast approximations |
| **MIP** | CPLEX, HiGHS | Proven optimality, small-medium instances |
| **CP** | CPLEX CP Optimizer, OR-Tools, XCSP3 | Complex constraints, scheduling |

---

## Metaheuristics

Fast, scalable solvers for large instances where optimal solutions aren't required.

| Solver | Name | Description |
|--------|------|-------------|
| **GA** | `ga` | Genetic Algorithm - good balance of quality and speed |
| **MA** | `ma` | Memetic Algorithm - GA with local search intensification |
| **VNS** | `vns` | Variable Neighborhood Search - systematic local search |
| **PSO** | `pso` | Particle Swarm Optimization - swarm-based metaheuristic |
| **LS** | `ls` | Local Search - fast improvement heuristic |

### Parameters

#### Iteration Budget

| Parameter | Type | Applies To | Description |
|-----------|------|------------|-------------|
| `iterMax` | int | GA, MA, VNS, PSO | Maximum iterations for main loop |

#### GA/MA Feasibility Controls

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `feasibleOnly` | bool | `true` | Enforce feasibility during decoding |
| `infeasiblePenalty` | float | `1000.0` | Penalty weight for infeasible solutions |
| `unservedPenalty` | float | `10000.0` | Penalty per unserved client |

### Example

```python
solver = routing.create_solver("ga", problem)
solver.set_param_int("iterMax", 10000)
solver.set_param_bool("feasibleOnly", True)
solver.set_param_float("infeasiblePenalty", 500.0)

if solver.solve(60.0):  # 60 second timeout
    print(f"Best cost: {solver.get_objective_value():.2f}")
```

---

## MIP (Mixed-Integer Programming)

Exact methods that can prove optimality for small-medium instances.

| Solver | Name | Backend | License |
|--------|------|---------|---------|
| **CPLEX MIP** | `mip/cplex` | IBM CPLEX | Commercial |
| **HiGHS MIP** | `mip/highs` | HiGHS | Open-source (MIT) |

### Example

```python
# Open-source solver
solver = routing.create_solver("mip/highs", problem)
solver.solve(300.0)  # 5 minute time limit

if solver.is_optimal():
    print("Proven optimal!")
```

---

## CP (Constraint Programming)

Powerful for problems with complex constraints like time windows and precedences.

| Solver | Name | Backend | License |
|--------|------|---------|---------|
| **CPLEX CP** | `cp/cplex` | IBM CP Optimizer | Commercial |
| **OR-Tools** | `cp/ortools` | Google OR-Tools CP-SAT | Open-source (Apache 2.0) |
| **XCSP3** | `cp/xcsp3` | [XCSP3](https://www.xcsp.org/) Format | Open standard |

### XCSP3 Integration

:::{admonition} XCSP3 - XML-based CP Format
:class: tip

[XCSP3](https://www.xcsp.org/) is an XML-based format for representing constraint satisfaction and optimization problems. The Routing library can:

- **Export** problems to XCSP3 format for use with any XCSP3-compatible solver
- **Solve** using external XCSP3 solvers like [ACE](https://github.com/xcsp3team/ACE), [Choco](https://choco-solver.org/), or [PicatSAT](http://picat-lang.org/)

This enables interoperability with the broader constraint programming ecosystem.
:::

**Key Benefits of XCSP3:**

- **Solver Independence**: Export once, solve with any XCSP3 solver
- **Benchmarking**: Compare different CP solvers on the same model
- **Research**: Standard format for academic publications
- **Competitions**: Official format for [XCSP competitions](https://www.xcsp.org/competitions/)

### Example: Export to XCSP3

```python
# Create solver with XCSP3 backend
solver = routing.create_solver("cp/xcsp3", problem)

# Export model to file
solver.export_model("routing_problem.xml")

# Or solve directly if an XCSP3 solver is configured
solver.solve(60.0)
```

### Example: OR-Tools CP-SAT

```python
solver = routing.create_solver("cp/ortools", problem)
solver.solve(120.0)  # 2 minute time limit

print(f"Objective: {solver.get_objective_value()}")
print(solver.get_stats())
```

---

## Solver Selection Guide

```{mermaid}
flowchart TD
    A[Start] --> B{Instance size?}
    B -->|Small < 50| C{Need optimality?}
    B -->|Medium 50-200| D{Time budget?}
    B -->|Large > 200| E[Metaheuristic]

    C -->|Yes| F[MIP or CP]
    C -->|No| G[Metaheuristic]

    D -->|Minutes| H[CP or MIP]
    D -->|Seconds| I[Metaheuristic]

    F --> J{Complex constraints?}
    J -->|Yes| K[CP]
    J -->|No| L[MIP]

    E --> M[GA or MA]
    G --> M
    I --> M
```

### Quick Recommendations

| Scenario | Recommended Solver |
|----------|-------------------|
| Quick solution needed | `ga` with low `iterMax` |
| Best quality, large instance | `ma` with high `iterMax` |
| Proven optimality needed | `mip/cplex` or `mip/highs` |
| Complex time windows | `cp/ortools` or `cp/cplex` |
| Benchmarking/Research | `cp/xcsp3` for export |
| Open-source only | `mip/highs`, `cp/ortools` |

---

## See Also

- [Installation](installation.md) - Installing solver backends
- [Benchmarks](benchmarks.md) - Performance comparisons
- [XCSP3 Website](https://www.xcsp.org/) - Official XCSP3 documentation
