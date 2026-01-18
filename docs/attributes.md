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
| **VRPTWTD** | All above + `Synced` |
| **PDVRP** | `GeoNode`, `Pickup`, `Delivery`, `Stock` |
| **TOP** | `GeoNode`, `Profiter`, `Rendezvous` |

---

## Usage

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

# Client
client = problem.add_client(1)
client.add_attribute(Attribute.GEONODE, x=10, y=20)
client.add_attribute(Attribute.CONSUMER, demand=5)
client.add_attribute(Attribute.RENDEZVOUS, open=0, close=200)
client.add_attribute(Attribute.SERVICE_QUERY, service_time=10)

# Vehicle
vehicle = problem.add_vehicle(0)
vehicle.add_attribute(Attribute.STOCK, capacity=100)

# Solve (attributes auto-enabled!)
solution = routing.solve(problem, "ga", timeout=30)
```

**Attribute Parameters:**

| Enum | Parameters |
|------|------------|
| `Attribute.GEONODE` | `x`, `y` |
| `Attribute.CONSUMER` | `demand` |
| `Attribute.STOCK` | `capacity` |
| `Attribute.RENDEZVOUS` | `open`, `close` |
| `Attribute.SERVICE_QUERY` | `service_time` |
| `Attribute.PROFITER` | `profit` |
| `Attribute.PICKUP` | `demand` |
| `Attribute.DELIVERY` | `demand` |

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
