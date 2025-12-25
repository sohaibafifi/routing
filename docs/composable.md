# Composable Attribute System API

The Composable Attribute System allows runtime composition of vehicle routing problem features without inheritance hierarchies. Instead of creating a new class for each problem variant (VRP → CVRP → CVRPTW), you enable attributes at runtime.

## Quick Start

```cpp
#include <plugins/PluginBundle.hpp>
#include <plugins/attributes/ComposableCorePlugin/ComposableProblem.hpp>
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>
#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>
#include <plugins/attributes/TimeWindowPlugin/Rendezvous.hpp>

int main() {
    // Register and initialize plugins
    routing::plugins::registerCorePlugins();
    routing::PluginRegistry::instance().initializeAll();

    // Create a composable problem
    routing::ComposableProblem problem;

    // Enable CVRPTW attributes
    problem.enableAttributes<
        routing::attributes::GeoNode,
        routing::attributes::Consumer,
        routing::attributes::Stock,
        routing::attributes::Rendezvous,
        routing::attributes::ServiceQuery
    >();

    // Add depot
    auto* depot = problem.addDepot(0);
    depot->addAttribute<routing::attributes::GeoNode>(0.0, 0.0);
    depot->addAttribute<routing::attributes::Rendezvous>(0, 1000);

    // Add vehicles
    for (int v = 0; v < 3; ++v) {
        auto* vehicle = problem.addVehicle(v);
        vehicle->addAttribute<routing::attributes::Stock>(100);
    }

    // Add clients
    auto* client = problem.addClient(1);
    client->addAttribute<routing::attributes::GeoNode>(10.0, 20.0);
    client->addAttribute<routing::attributes::Consumer>(15);
    client->addAttribute<routing::attributes::Rendezvous>(50, 200);
    client->addAttribute<routing::attributes::ServiceQuery>(10);

    // Constraint generators are auto-activated based on enabled attributes
    auto generators = problem.getActiveGenerators();
    // Returns: RoutingConstraintGenerator, CapacityConstraintGenerator, TimeWindowConstraintGenerator

    // Cleanup
    routing::PluginRegistry::instance().shutdownAll();
    return 0;
}
```

## Core Classes

### ComposableProblem

The main problem class for composable modeling.

```cpp
namespace routing {
    class ComposableProblem : public Problem {
    public:
        // Enable specific attributes for this problem
        template<typename... Attrs>
        void enableAttributes();

        // Check if an attribute is enabled
        template<typename T>
        bool isAttributeEnabled() const;

        // Entity management
        ComposableClient* addClient(unsigned id);
        ComposableVehicle* addVehicle(unsigned id);
        ComposableDepot* addDepot(unsigned id);

        // Access entities
        std::vector<ComposableClient*> getComposableClients() const;
        std::vector<ComposableVehicle*> getComposableVehicles() const;
        ComposableDepot* getComposableDepot() const;

        // Get active generators for enabled attributes
        std::vector<IConstraintGenerator*> getActiveGenerators() const;

        // Problem statistics
        size_t numClients() const;
        size_t numVehicles() const;
    };
}
```

### ComposableEntity

Base class for entities with runtime attributes.

```cpp
namespace routing {
    class ComposableEntity : public virtual Model {
    public:
        // Add an attribute to this entity
        template<typename T, typename... Args>
        T* addAttribute(Args&&... args);

        // Get an attribute (throws if not present)
        template<typename T>
        T& getAttribute();
        template<typename T>
        const T& getAttribute() const;

        // Try to get an attribute (returns nullptr if not present)
        template<typename T>
        T* tryGetAttribute();
        template<typename T>
        const T* tryGetAttribute() const;

        // Check if entity has an attribute
        template<typename T>
        bool hasAttribute() const;

        // Get all attribute type IDs
        std::vector<AttributeTypeId> getAttributeTypes() const;
    };

    // Specialized entity types
    class ComposableClient : public ComposableEntity, public models::Client {};
    class ComposableVehicle : public ComposableEntity, public models::Vehicle {};
    class ComposableDepot : public ComposableEntity, public models::Depot {};
}
```

## Available Attributes

### Routing Attributes

| Attribute | Location | Description |
|-----------|----------|-------------|
| `GeoNode` | `plugins/attributes/RoutingPlugin/GeoNode.hpp` | Geographic coordinates (x, y) |

```cpp
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>

client->addAttribute<routing::attributes::GeoNode>(x, y);
auto* geo = client->tryGetAttribute<routing::attributes::GeoNode>();
double x = geo->getX();
double y = geo->getY();
```

### Capacity Attributes

| Attribute | Location | Description |
|-----------|----------|-------------|
| `Consumer` | `plugins/attributes/CapacityPlugin/Consumer.hpp` | Demand at a node |
| `Stock` | `plugins/attributes/CapacityPlugin/Stock.hpp` | Vehicle capacity |

```cpp
#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>

client->addAttribute<routing::attributes::Consumer>(15);  // demand of 15
vehicle->addAttribute<routing::attributes::Stock>(100);   // capacity of 100
```

### Time Window Attributes

| Attribute | Location | Description |
|-----------|----------|-------------|
| `Rendezvous` | `plugins/attributes/TimeWindowPlugin/Rendezvous.hpp` | Hard time window bounds |
| `ServiceQuery` | `plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp` | Service time at node |
| `SoftTimeWindows` | `plugins/attributes/TimeWindowPlugin/SoftTimeWindows.hpp` | Soft TW with penalties |
| `Appointment` | `plugins/attributes/TimeWindowPlugin/Appointment.hpp` | Fixed appointment times |
| `Service` | `plugins/attributes/TimeWindowPlugin/Service.hpp` | Service characteristics |

```cpp
#include <plugins/attributes/TimeWindowPlugin/Rendezvous.hpp>
#include <plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp>

client->addAttribute<routing::attributes::Rendezvous>(50, 200);  // TW [50, 200]
client->addAttribute<routing::attributes::ServiceQuery>(10);     // service time 10
```

### Profit Attributes

| Attribute | Location | Description |
|-----------|----------|-------------|
| `Profiter` | `plugins/attributes/ProfitPlugin/Profiter.hpp` | Profit for visiting node |

```cpp
#include <plugins/attributes/ProfitPlugin/Profiter.hpp>

client->addAttribute<routing::attributes::Profiter>(100.0);  // profit of 100
```

### Pickup & Delivery Attributes

| Attribute | Location | Description |
|-----------|----------|-------------|
| `Pickup` | `plugins/attributes/PickupDeliveryPlugin/Pickup.hpp` | Pickup node |
| `Delivery` | `plugins/attributes/PickupDeliveryPlugin/Delivery.hpp` | Delivery node |

```cpp
#include <plugins/attributes/PickupDeliveryPlugin/Pickup.hpp>
#include <plugins/attributes/PickupDeliveryPlugin/Delivery.hpp>

pickupClient->addAttribute<routing::attributes::Pickup>(10);    // pickup 10 units
deliveryClient->addAttribute<routing::attributes::Delivery>(10); // deliver 10 units
```

### Synchronization Attributes

| Attribute | Location | Description |
|-----------|----------|-------------|
| `Synced` | `plugins/attributes/SyncPlugin/Synced.hpp` | Temporal synchronization |

```cpp
#include <plugins/attributes/SyncPlugin/Synced.hpp>

// Two clients that must be visited at the same time
client1->addAttribute<routing::attributes::Synced>(0);  // sync group 0
client2->addAttribute<routing::attributes::Synced>(0);  // sync group 0
```

## Constraint Generators

Constraint generators are automatically activated when their required attributes are enabled.

| Generator | Required Attributes | Priority | Purpose |
|-----------|---------------------|----------|---------|
| `RoutingConstraintGenerator` | `GeoNode` | 10 | Base routing constraints |
| `CapacityConstraintGenerator` | `Consumer`, `Stock` | 50 | Vehicle capacity |
| `TimeWindowConstraintGenerator` | `Rendezvous` | 60 | Hard time windows |
| `SoftTimeWindowGenerator` | `SoftTimeWindows` | 61 | Soft time windows |
| `PickupDeliveryConstraintGenerator` | `Pickup`, `Delivery` | 55 | P&D precedence |
| `SyncConstraintGenerator` | `Synced` | 70 | Temporal synchronization |
| `ProfitObjectiveGenerator` | `Profiter` | 1000 | Maximize profit |

### How Auto-Activation Works

When you call `problem.enableAttributes<A, B, C>()`:

1. The problem registers attribute types A, B, C as enabled
2. The PluginRegistry is queried for all registered generators
3. Each generator's `isApplicable(enabledAttrs)` method is checked
4. Matching generators are sorted by priority (lower = earlier)
5. Generators are stored in `activeGenerators_`

```cpp
// Get active generators
auto generators = problem.getActiveGenerators();

// Manual check
for (auto* gen : generators) {
    std::cout << gen->name() << " (priority: " << gen->priority() << ")" << std::endl;
}
```

## Creating Custom Attributes

### Step 1: Define the Attribute

```cpp
// plugins/attributes/MyPlugin/MyAttribute.hpp
#pragma once

#include "core/interfaces/IAttribute.hpp"

namespace routing::attributes {

class MyAttribute : public Attribute<MyAttribute> {
public:
    explicit MyAttribute(double value) : value_(value) {}

    double getValue() const { return value_; }
    void setValue(double v) { value_ = v; }

private:
    double value_;
};

} // namespace routing::attributes
```

### Step 2: Create a Constraint Generator (Optional)

```cpp
// plugins/attributes/MyPlugin/MyConstraintGenerator.hpp
#pragma once

#include "core/interfaces/IConstraintGenerator.hpp"
#include "MyAttribute.hpp"

namespace routing::constraints {

class MyConstraintGenerator : public IConstraintGenerator {
public:
    std::string name() const override { return "MyConstraintGenerator"; }
    int priority() const override { return 100; }

    std::set<AttributeTypeId> requiredAttributes() const override {
        return { routing::attributes::MyAttribute::staticTypeId() };
    }

#ifdef CPLEX_FOUND
    void addVariables(ComposableProblem* problem, IloEnv& env, IloModel& model) override {
        // Add MIP variables
    }

    void addConstraints(ComposableProblem* problem, IloEnv& env, IloModel& model) override {
        // Add MIP constraints
    }
#endif
};

} // namespace routing::constraints
```

### Step 3: Create a Plugin to Register

```cpp
// plugins/attributes/MyPlugin/MyPlugin.hpp
#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "MyConstraintGenerator.hpp"

namespace routing::plugins {

class MyPlugin : public IPlugin {
public:
    std::string name() const override { return "MyPlugin"; }
    PluginType type() const override { return PluginType::Attribute; }

    void initialize(PluginRegistry& registry) override {
        if (!registry.hasGenerator("MyConstraintGenerator")) {
            registry.registerGenerator(
                std::make_unique<constraints::MyConstraintGenerator>());
        }
    }
};

} // namespace routing::plugins
```

### Step 4: Register in PluginBundle

Add to `plugins/PluginBundle.cpp`:

```cpp
#include "plugins/attributes/MyPlugin/MyPlugin.hpp"

void registerCorePlugins() {
    auto& registry = PluginRegistry::instance();
    // ... existing plugins ...
    registry.registerPlugin(std::make_unique<MyPlugin>());
}
```

## Problem Types with Composable Attributes

| Problem | Enabled Attributes |
|---------|-------------------|
| VRP | `GeoNode` |
| CVRP | `GeoNode`, `Consumer`, `Stock` |
| VRPTW | `GeoNode`, `Rendezvous`, `ServiceQuery` |
| CVRPTW | `GeoNode`, `Consumer`, `Stock`, `Rendezvous`, `ServiceQuery` |
| TOP | `GeoNode`, `Consumer`, `Stock`, `Profiter` |
| TOPTW | `GeoNode`, `Consumer`, `Stock`, `Profiter`, `Rendezvous`, `ServiceQuery` |
| PDVRP | `GeoNode`, `Consumer`, `Stock`, `Pickup`, `Delivery` |
| VRPTWTD | `GeoNode`, `Consumer`, `Stock`, `Rendezvous`, `ServiceQuery`, `Synced` |

## Using with Solvers

### MIP Solver

```cpp
#include <plugins/solvers/MIPSolverPlugin/MIPSolver.hpp>

routing::ComposableProblem problem;
problem.enableAttributes<GeoNode, Consumer, Stock>();

// Add entities...

routing::MIPSolver solver(&problem);
bool solved = solver.solve(60.0);  // 60 second timeout

if (solved) {
    auto* solution = solver.getSolution();
    solution->print(std::cout);
}
```

### Using PluginRegistry Factory

```cpp
routing::plugins::registerCorePlugins();
auto& registry = routing::PluginRegistry::instance();
registry.initializeAll();

routing::ComposableProblem problem;
// ... configure problem ...

// Create solver via registry
auto solver = registry.createSolver("mip", &problem);
solver->solve(60.0);
```

## Best Practices

1. **Enable Only Needed Attributes**: More attributes = more constraints = slower solving
2. **Use tryGetAttribute for Safety**: Returns nullptr if attribute not present
3. **Check Active Generators**: Verify expected generators are activated
4. **Initialize Plugins Once**: Call `registerCorePlugins()` and `initializeAll()` at startup
5. **Shutdown at Exit**: Call `shutdownAll()` before program exit

## Example: Complete CVRPTW Program

See `examples/cvrptw.cpp` and `examples/top.cpp` for CLI examples using composable readers.

```bash
./build/examples/example_cvrptw -i data/CVRPTW/Solomon/10/c101.txt
./build/examples/example_top -i data/CVRP/A/A-n32-k5.vrp
```
