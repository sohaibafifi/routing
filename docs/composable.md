# Composable Attribute System

Runtime composition of VRP features without inheritance hierarchies.

## Quick Start

### Python

```python
import routing
from routing.constants import Attribute

routing.init()

problem = routing.Problem()

# Depot
depot = problem.add_depot(0)
depot.add_attribute(Attribute.GEONODE, x=0, y=0)
depot.add_attribute(Attribute.RENDEZVOUS, open=0, close=1000)

# Vehicles
for v in range(3):
    vehicle = problem.add_vehicle(v)
    vehicle.add_attribute(Attribute.STOCK, capacity=100)

# Clients
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, x=10, y=20)
client.add_attribute(Attribute.CONSUMER, demand=15)
client.add_attribute(Attribute.RENDEZVOUS, open=50, close=200)
client.add_attribute(Attribute.SERVICE_QUERY, service_time=10)

# Solve (constraints auto-activated!)
solution = routing.solve(problem, "ga", timeout=30)
```

### C++

```cpp
#include <routing/routing.hpp>

int main() {
    routing::Problem problem;

    // Depot
    auto* depot = problem.addDepot(0);
    depot->addAttribute<routing::attributes::GeoNode>(0.0, 0.0);
    depot->addAttribute<routing::attributes::Rendezvous>(0, 1000);

    // Vehicles
    for (int v = 0; v < 3; ++v) {
        auto* vehicle = problem.addVehicle(v);
        vehicle->addAttribute<routing::attributes::Stock>(100);
    }

    // Clients
    auto* client = problem.addClient(1);
    client->addAttribute<routing::attributes::GeoNode>(10.0, 20.0);
    client->addAttribute<routing::attributes::Consumer>(15);
    client->addAttribute<routing::attributes::Rendezvous>(50, 200);
    client->addAttribute<routing::attributes::ServiceQuery>(10);

    // Solve
    routing::GASolver solver(&problem);
    solver.solve(30.0);

    return 0;
}
```

---

## Available Attributes

| Attribute | Entity | Description |
|-----------|--------|-------------|
| `GeoNode` | Any | Coordinates (x, y) |
| `Consumer` | Client | Demand |
| `Stock` | Vehicle | Capacity |
| `Rendezvous` | Any | Time window (open, close) |
| `ServiceQuery` | Client | Service time |
| `Profiter` | Client | Profit (TOP) |
| `Pickup` | Client | Pickup demand |
| `Delivery` | Client | Delivery demand |
| `Synced` | Client | Synchronization |

---

## Problem Types

| Problem | Attributes |
|---------|-----------|
| **TSP** | `GeoNode` |
| **CVRP** | `GeoNode`, `Consumer`, `Stock` |
| **VRPTW** | `GeoNode`, `Rendezvous`, `ServiceQuery` |
| **CVRPTW** | `GeoNode`, `Consumer`, `Stock`, `Rendezvous`, `ServiceQuery` |
| **TOP** | `GeoNode`, `Profiter`, `Rendezvous` |
| **PDVRP** | `GeoNode`, `Pickup`, `Delivery`, `Stock` |

---

## How It Works

1. **Add attributes** to entities
2. **Constraints auto-activate** based on attributes:
   - `GeoNode` → Routing/distance
   - `Consumer` + `Stock` → Capacity constraints
   - `Rendezvous` → Time windows
3. **Solve** with any solver

---

## Custom Attributes (C++)

```cpp
// Define attribute
struct MyAttribute : public Attribute<MyAttribute> {
    double value;
    explicit MyAttribute(double v) : value(v) {}
};

// Use it
client->addAttribute<MyAttribute>(42.0);
```

See [Plugins](plugins.md) for full custom attribute guide.
