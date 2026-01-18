# Solvers

The Routing library provides multiple solver backends for different needs.

---

## Overview

| Category | Solvers | Best For |
|----------|---------|----------|
|  **Metaheuristics** | GA, MA, VNS, PSO, LS, ALNS | Large instances, fast approximations |
|  **MIP** | CPLEX, HiGHS | Proven optimality, small-medium instances |
|  **CP** | CPLEX CP Optimizer, OR-Tools, XCSP3 | Complex constraints, scheduling |

---

## Metaheuristics

Fast, scalable solvers for large instances where optimal solutions aren't required.

| Solver | Name | Description |
|--------|------|-------------|
|  **GA** | `ga` | Genetic Algorithm - good balance of quality and speed |
|  **MA** | `ma` | Memetic Algorithm - GA with local search intensification |
|  **VNS** | `vns` | Variable Neighborhood Search - systematic local search |
|  **PSO** | `pso` | Particle Swarm Optimization - swarm-based metaheuristic |
|  **LS** | `ls` | Local Search - fast improvement heuristic |
|  **ALNS** | `alns` | Adaptive Large Neighborhood Search - multi-destroy/repair with adaptive weights |

### Parameters

#### Iteration Budget

| Parameter | Type | Applies To | Description |
|-----------|------|------------|-------------|
|  `iterMax` | int | GA, MA, VNS, PSO, ALNS | Maximum iterations for main loop |

#### GA/MA Feasibility Controls

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
|  `feasibleOnly` | bool | `true` | Enforce feasibility during decoding |
|  `infeasiblePenalty` | float | `1000.0` | Penalty weight for infeasible solutions |
|  `unservedPenalty` | float | `10000.0` | Penalty per unserved client |

#### ALNS Adaptive Parameters

ALNS uses adaptive operator selection that learns which destroy/repair combinations work best during search.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
|  `reactionFactor` | float | `0.1` | Weight update reaction factor (0-1) |
|  `decayFactor` | float | `0.8` | Weight decay factor per segment (0-1) |
|  `temperature` | float | `100.0` | Initial temperature for simulated annealing |
|  `coolingRate` | float | `0.9995` | Temperature cooling rate per iteration |
|  `segmentSize` | int | `100` | Iterations between weight updates |
|  `minTemperature` | float | `0.01` | Minimum temperature for acceptance |

**ALNS Destroy Operators:**
- **RandomRemoval**: Removes random subset of clients (configurable fraction)
- **WorstRemoval**: Removes clients with highest cost impact (with randomness)
- **ShawRemoval**: Removes similar clients (based on distance, demand, time windows)
- **RouteRemoval**: Removes entire routes from solution

**ALNS Repair Operators:**
- **GreedyRepair**: Re-inserts clients using greedy best-insertion
- **RegretRepair**: Re-inserts using regret-k heuristic (balances multiple good positions)
- **RandomRepair**: Randomly re-inserts clients

### Examples

#### GA Example

```python
import routing
from routing.constants import Attribute

routing.init()

problem = routing.Problem()
# ... add entities with attributes ...

solution = routing.solve(problem, "ga", timeout=60)
print(f"Cost: {solution.cost:.2f}")
```

#### ALNS Example

```python
solution = routing.solve(problem, "alns", timeout=60)
print(f"Cost: {solution.cost:.2f}")
```

---

## MIP (Mixed-Integer Programming)

Exact methods that can prove optimality for small-medium instances.

| Solver | Name | Backend | License |
|--------|------|---------|---------|
|  **CPLEX MIP** | `mip/cplex` | IBM CPLEX | Commercial |
|  **HiGHS MIP** | `mip/highs` | HiGHS | Open-source (MIT) |

### Example

```python
# Open-source MIP solver
solution = routing.solve(problem, "mip/highs", timeout=300)

if solution:
    print(f"Cost: {solution.cost:.2f}")
```

---

## CP (Constraint Programming)

Powerful for problems with complex constraints like time windows and precedences.

| Solver | Name | Backend | License |
|--------|------|---------|---------|
|  **CPLEX CP** | `cp/cpo` | IBM CP Optimizer | Commercial |
|  **OR-Tools** | `cp/ortools` | Google OR-Tools CP-SAT | Open-source (Apache 2.0) |
|  **XCSP3** | `cp/ace` | [XCSP3](https://www.xcsp.org/) Format | Open standard |

### XCSP3 Integration

:::{admonition} XCSP3 - XML-based CP Format
:class: tip

[XCSP3](https://www.xcsp.org/) is an XML-based format for representing constraint satisfaction and optimization problems. The Routing library can:

- **Export** problems to XCSP3 format for use with any XCSP3-compatible solver
- **Solve** using external XCSP3 solvers like [ACE](https://github.com/xcsp3team/ACE), [Choco](https://choco-solver.org/), or [PicatSAT](http://picat-lang.org/)

This enables interoperability with broader constraint programming ecosystem.
:::

**Key Benefits of XCSP3:**

- **Solver Independence**: Export once, solve with any XCSP3 solver
- **Benchmarking**: Compare different CP solvers on same model
- **Research**: Standard format for academic publications
- **Competitions**: Official format for [XCSP competitions](https://www.xcsp.org/competitions/)

### Example: Export to XCSP3

```python
solver = routing.create_solver("cp/ace", problem)
solver.export_model("routing_problem.xml")
```

### Example: OR-Tools CP-SAT

```python
solution = routing.solve(problem, "cp/ortools", timeout=120)
print(f"Cost: {solution.cost:.2f}")
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

    E --> M{Adaptive search?}
    M -->|Yes| N[ALNS]
    M -->|No| O[GA or MA]
    G --> M
    I --> M
```

### Quick Recommendations

| Scenario | Recommended Solver |
|----------|-------------------|
| Quick solution needed | `ga` with low `iterMax` |
| Best quality, large instance | `ma` or `alns` with high `iterMax` |
| Adaptive exploration of neighborhoods | `alns` with custom parameters |
| Proven optimality needed | `mip/cplex` or `mip/highs` |
| Complex time windows | `cp/ortools` or `cp/cpo` |
| Benchmarking/Research | `cp/ace` for export |
| Open-source only | `mip/highs`, `cp/ortools`, `alns` |

### ALNS vs Other Metaheuristics

| Aspect | ALNS | GA/MA/PSO | VNS |
|---------|--------|-------------|-----|
| **Neighborhood Exploration** | Multiple adaptive | Population-based | Systematic |
| **Diversification** | Destroy operators | Crossover/mutation | Shaking |
| **Intensification** | Repair operators | Selection/local search | Best improvement |
| **Adaptivity** | Dynamic operator weights | Parameter tuning | Fixed |
| **Best For** | Rich neighborhood structure | Diverse solutions | Local improvement |
| **Complexity** | Medium | Low-Medium | Low |

---

## See Also

- [Installation](installation.md) - Installing solver backends
- [Benchmarks](benchmarks.md) - Performance comparisons
- [XCSP3 Website](https://www.xcsp.org/) - Official XCSP3 documentation
