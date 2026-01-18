# Attributes

Attributes define problem features and automatically enable matching constraint generators and evaluators.

## Available Attributes

| Attribute | Entity | Description |
|-----------|--------|-------------|
| **GeoNode** | Node | x, y coordinates for distance calculation |
| **Consumer** | Client | Demand at a client |
| **Stock** | Vehicle | Vehicle capacity |
| **Rendezvous** | Node | Time window bounds (earliest, latest) |
| **ServiceQuery** | Client | Service time at a node |
| **Profiter** | Client | Profit value for orienteering problems (TOP) |
| **Pickup** | Client | Pickup demand for pickup & delivery |
| **Delivery** | Client | Delivery demand for pickup & delivery |
| **Synced** | Client | Temporal synchronization constraints |
| **SoftTimeWindows** | Client | Penalty-based soft time windows |

---

## Problem Variants by Attributes

| Problem | Required Attributes |
|---------|---------------------|
| **TSP** | `GeoNode` |
| **VRP** | `GeoNode` |
| **CVRP** | `GeoNode`, `Consumer`, `Stock` |
| **VRPTW** | `GeoNode`, `Rendezvous`, `ServiceQuery` |
| **CVRPTW** | `GeoNode`, `Consumer`, `Stock`, `Rendezvous`, `ServiceQuery` |
| **TOP** | `GeoNode`, `Profiter`, `Rendezvous` |
| **PDVRP** | `GeoNode`, `Pickup`, `Delivery`, `Stock` |
| **VRPTWTD** | All above + `Synced` |

---

## Usage

### Python - Generic Attribute API

Python uses a generic `add_attribute()` method for all attributes:

```python
import routing

routing.init()

# Create problem
problem = routing.Problem()

# Add depot and set its attributes
depot = problem.add_depot(0)
depot.add_attribute("GeoNode", 0.0, 0.0)           # Location
depot.add_attribute("Rendezvous", 0.0, 1000.0)     # Time window

# Add client and set its attributes
client = problem.add_client(1)
client.add_attribute("GeoNode", 10.0, 20.0)        # Location
client.add_attribute("Consumer", 5)                 # Demand
client.add_attribute("Rendezvous", 0.0, 200.0)     # Time window
client.add_attribute("ServiceQuery", 10.0)          # Service time

# Add vehicle and set its attributes
vehicle = problem.add_vehicle(0)
vehicle.add_attribute("Stock", 100)                 # Capacity

# Attributes are automatically enabled when added!
# Solve
solver = routing.create_solver("ga", problem)
solver.solve(30.0)
```

**Generic Attribute API - Syntax:**

| Attribute | Syntax |
|-----------|--------|
| `GeoNode` | `entity.add_attribute("GeoNode", x, y)` |
| `Consumer` | `client.add_attribute("Consumer", demand)` |
| `Stock` | `vehicle.add_attribute("Stock", capacity)` |
| `Rendezvous` | `entity.add_attribute("Rendezvous", open, close)` |
| `ServiceQuery` | `client.add_attribute("ServiceQuery", time)` |
| `Profiter` | `client.add_attribute("Profiter", profit)` |
| `Pickup` | `client.add_attribute("Pickup", demand)` |
| `Delivery` | `client.add_attribute("Delivery", demand)` |

### Python - ProblemBuilder (High-Level)

The `ProblemBuilder` provides a fluent interface using the generic API internally:

```python
from routing.problem import ProblemBuilder

problem = (ProblemBuilder()
    .with_depot(0, 0, tw_open=0, tw_close=1000)
    .add_client(x=10, y=20, demand=5, tw_open=0, tw_close=200, service_time=10)
    .add_vehicles(count=2, capacity=20)
    .build())  # Attributes are automatically enabled

# Ready to solve - no enable_attributes() needed!
solution = routing.solve(problem, "ga", timeout=30)
```

### C++

In C++, you use template methods to add attributes. Attributes are automatically enabled when added:

```cpp
// Add depot with attributes
auto* depot = problem.addDepot(0);
depot->addAttribute<GeoNode>(0.0, 0.0);           // Auto-enables GeoNode
depot->addAttribute<Rendezvous>(0.0, 1000.0);     // Auto-enables Rendezvous

// Add client with attributes
auto* client = problem.addClient(1);
client->addAttribute<GeoNode>(10.0, 20.0);        // Coordinates
client->addAttribute<Consumer>(5);                 // Auto-enables Consumer
client->addAttribute<Rendezvous>(0.0, 200.0);     // Time window
client->addAttribute<ServiceQuery>(10.0);          // Auto-enables ServiceQuery

// Add vehicle with attributes
auto* vehicle = problem.addVehicle(0);
vehicle->addAttribute<Stock>(20);                  // Auto-enables Stock

// Attributes are automatically enabled!
// You can still explicitly enable if needed:
// problem.enableAttributes<GeoNode, Consumer, Stock>();
```

---

## How Attributes Work

### Automatic Enabling

When you add an attribute to an entity using `add_attribute()`, the system automatically:

1. **Stores the attribute data** on the entity
2. **Auto-enables the attribute type** on the problem (if not already enabled)
3. **Activates constraint generators** that depend on that attribute
4. **Activates evaluators** for solution quality assessment

```
entity.add_attribute() → Auto-Enable Attribute Type → Activate Generators → Add Constraints
         ↓
    GeoNode              → RoutingGenerator         → Distance objective
    Consumer             → CapacityGenerator        → Load ≤ Capacity
    Stock                → CapacityGenerator        → Vehicle capacity limit
    Rendezvous           → TimeWindowGenerator      → Arrival ∈ [open, close]
    ServiceQuery         → TimeWindowGenerator      → Service time in scheduling
```

**Note:** You can still explicitly pre-enable attributes if needed:
```python
# Optional: Pre-enable before adding entities
problem.enable_attributes(["GeoNode", "Consumer", "Stock"])
```

---

## Custom Attributes

You can define custom attributes using the CRTP pattern:

```cpp
struct MyAttribute : public Attribute<MyAttribute> {
    static constexpr const char* name() { return "MyAttribute"; }

    double value;

    explicit MyAttribute(double v) : value(v) {}
};
```

See [Composable System](composable.md) for more details on extending the attribute system.
