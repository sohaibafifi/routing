# Ri7la: Composable VRP Library

<p align="center">
  <img src="docs/assets/logo.svg" alt="Ri7la logo" width="200"/>
</p>

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/85e69139d552469fa1c0b0f1b098f60f)](https://app.codacy.com/manual/me_183/routing?utm_source=github.com&utm_medium=referral&utm_content=sohaibafifi/rihla&utm_campaign=Badge_Grade_Dashboard)

**Ri7la** is a modern, modular C++ library for solving Vehicle Routing Problems (VRP). It features a **Composable Architecture** that separates problem definition (Attributes) from solution methods (Solvers), allowing you to mix and match components to solve complex variants efficiently.

**Ri7la** name is derived from the Arabic word "رحلة" meaning "journey" or "trip", reflecting its focus on routing and logistics optimization. The 7 represents the Arabic letter "ح" (Haa), pronounced as a hard "H", not like the one in my name "Sohaib" ;).
## Project Ecosystem

### 1. Composable Problem Definition (Attributes)
Define your problem by composing standard attributes. The library handles the complex interactions between them.

*   **Capacity**: Demand and vehicle capacity handling (`plugins/attributes/CapacityPlugin`).
*   **Time Windows**: Service times, arrival windows, and waiting times (`plugins/attributes/TimeWindowPlugin`).
*   **Pickup & Delivery**: Linked pickup and delivery requests (`plugins/attributes/PickupDeliveryPlugin`).
*   **Profit**: For variants like Team Orienteering or Prize Collecting VRP (`plugins/attributes/ProfitPlugin`).
*   **Synchronization**: Constraints between vehicles or nodes (`plugins/attributes/SyncPlugin`).

### 2. Multi-Paradigm Solvers
The library expects to support a wide range of exact and heuristic solvers:

**Exact Methods:**
*   **CP Optimizer**: Wrapper for IBM ILOG CP Optimizer (Constraint Programming).
*   **XCSP3**: Generates standard XCSP3 models for solvers like **ACE**, **CoSoCo**, or **Choco**.
*   **MIP**: Mathematical Programming backend (supports **Gurobi** and **CPLEX**).

**Metaheuristics & Heuristics:**
*   **Local Search (LS)**: Core local search framework (`plugins/solvers/LSSolverPlugin`).
*   **Genetic Algorithm (GA)**: Population-based evolution (`plugins/solvers/GASolverPlugin`).
*   **Memetic Algorithm (MA)**: Hybrid GA + Local Search (`plugins/solvers/MASolverPlugin`).
*   **Particle Swarm Optimization (PSO)**: Swarm intelligence (`plugins/solvers/PSOSolverPlugin`).
*   **Variable Neighborhood Search (VNS)**: Systematic neighborhood exploration (`plugins/solvers/VNSSolverPlugin`).

### 3. Neighborhoods & Operators
For heuristic solvers, efficient move operators are available:
*   **2-Opt**: Classic route improvement operator (`plugins/neighborhoods/TwoOptPlugin`).
*   **IDCH**: Iterated Destroy and Construct Heuristic (`plugins/neighborhoods/IDCHPlugin`).

## Python Extension

*   [**Python Extension**](python/pycsp3_extension/README.md): Documentation for the new **Python Composable API**, which allows modeling VRPs in Python and solving them via the XCSP3 ecosystem.

## Architecture Overview

```mermaid
---
config:
  layout: dagre
---
flowchart TB
 subgraph Problem_Layer["Problem Layer"]
    direction TB
        PL1["Attribute<br>Plugin"]
        PL2["Constraint<br>Plugin"]
        PL3["Evaluator<br>Plugin"]
        PL4["Propagator<br>Plugin"]
  end
 subgraph Solver_Layer["Solver Layer"]
    direction TB
        SL1["Meta-heuristics"]
        SL2["MIP"]
        SL3["CP"]
        SL4["Hybrid"]
  end
 subgraph MIP_Back["MIP Backend"]
    direction TB
        M1["CPLEX"]
        M2["GUROBI"]
        M3["HiGHS"]
  end
 subgraph CP_Back["CP Backend"]
    direction TB
        C1["CP-SAT"]
        C2["CPO"]
        C3["ACE"]
  end
 subgraph Opt_Backends["Optimization Backends"]
    direction TB
        MIP_Back
        CP_Back
  end
 subgraph Core_Lib["C++ Core Library"]
    direction TB
        Problem_Layer
        Solver_Layer
  end
    API["C++/Python API<br>"] --> Core_Lib
    Core_Lib --> Opt_Backends
    Problem_Layer --> Solver_Layer

     PL1:::problem
     PL2:::problem
     PL3:::problem
     PL4:::problem
     SL1:::solver
     SL2:::solver
     SL3:::solver
     SL4:::solver
    
     M1:::mip
     M2:::mip
     M3:::mip
     C1:::cp
     C2:::cp
     C3:::cp
     Problem_Layer:::layer
     Solver_Layer:::layer
     API:::python
     Opt_Backends:::layer
    classDef python fill:#FFD43B,stroke:#306998,stroke-width:2px,color:#306998,font-weight:bold
    classDef core fill:#f9f9f9,stroke:#333,stroke-width:2px,stroke-dasharray: 5 5
    classDef layer fill:#fff,stroke:#666,stroke-width:1px,color:#333
    classDef problem fill:#e1f5fe,stroke:#0277bd,stroke-width:1px,rx:5,ry:5
    classDef solver fill:#e8f5e9,stroke:#2e7d32,stroke-width:1px,rx:5,ry:5
    classDef mip fill:#fff3e0,stroke:#ef6c00,stroke-width:1px,rx:5,ry:5
    classDef cp fill:#f3e5f5,stroke:#7b1fa2,stroke-width:1px,rx:5,ry:5
    classDef subBackend fill:#fafafa,stroke:#999,stroke-width:1px,stroke-dasharray: 3 3
```

---

## Compilation

```bash
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### CMake Options

| Option | Default | Description |
| :--- | :--- | :--- |
| `ROUTING_BUILD_EXAMPLES` | `ON` | Build example executables. |
| `ROUTING_BUILD_XCSP3` | `ON` | Build the XCSP3 solver backend. |
| `ROUTING_BUILD_CPOPTIMIZER` | `ON` | Build CP Optimizer support (requires CPLEX). |
| `ROUTING_BUILD_GRB` | `ON` | Build Gurobi support (requires Gurobi). |
| `BUILD_TESTING` | `ON` | Build unit tests. |

## External References

This project leverages and integrates with top-tier optimization tools:

*   **[XCSP3](http://www.xcsp.org/)**: The standard XML format for CP models.
*   **[ACE](https://github.com/xcsp3team/ace)**: A state-of-the-art open-source CP solver.
*   **[CP Optimizer](https://www.ibm.com/products/ilog-cplex-optimization-studio/cplex-cp-optimizer)**: IBM's constraint programming solver.
*   **[Gurobi](https://www.gurobi.com/)**: Another mathematical programming solver.

## License
You are allowed to retrieve this project for research purposes as a member of a non-commercial and academic institution.
