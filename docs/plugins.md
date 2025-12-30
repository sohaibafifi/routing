# Plugin System API

The Plugin System provides a modular architecture for extending the routing framework with new solvers, neighborhoods, readers, and constraint generators.

## Quick Start

```cpp
#include <plugins/PluginBundle.hpp>
#include <core/PluginRegistry.hpp>

int main() {
    // Register all core plugins
    routing::plugins::registerCorePlugins();

    // Initialize all plugins (respects dependency order)
    auto& registry = routing::PluginRegistry::instance();
    registry.initializeAll();

    // Use registered components
    auto solver = registry.createSolver("mip", problem);
    solver->solve(60.0);

    // Cleanup
    registry.shutdownAll();
    return 0;
}
```

## Core Classes

### IPlugin

Base interface for all plugins.

```cpp
namespace routing {

enum class PluginType {
    Attribute,
    ConstraintGenerator,
    Evaluator,
    Solver,
    Neighborhood,
    Reader
};

class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Unique plugin identifier
    virtual std::string name() const = 0;

    // Semantic version
    virtual std::string version() const { return "1.0.0"; }

    // Human-readable description
    virtual std::string description() const { return ""; }

    // Plugin category
    virtual PluginType type() const = 0;

    // List of plugin names this depends on
    virtual std::vector<std::string> dependencies() const { return {}; }

    // Called during initializeAll() - register components here
    virtual void initialize(PluginRegistry& registry) = 0;

    // Called during shutdownAll()
    virtual void shutdown() {}
};

} // namespace routing
```

### PluginRegistry

Singleton registry for all plugins and components.

```cpp
namespace routing {

class PluginRegistry {
public:
    // Singleton access
    static PluginRegistry& instance();

    // Plugin lifecycle
    void registerPlugin(std::unique_ptr<IPlugin> plugin);
    void initializeAll();
    void shutdownAll();
    IPlugin* getPlugin(const std::string& name) const;

    // Constraint generator management
    void registerGenerator(std::unique_ptr<IConstraintGenerator> generator);
    IConstraintGenerator* getGenerator(const std::string& name) const;
    bool hasGenerator(const std::string& name) const;
    std::vector<IConstraintGenerator*> getGenerators(
        const std::set<AttributeTypeId>& enabledAttrs) const;
    std::vector<std::string> availableGenerators() const;

    // Solver factory
    using SolverFactory = std::function<std::unique_ptr<ISolver>(Problem*)>;
    void registerSolver(const std::string& name, SolverFactory factory);
    std::unique_ptr<ISolver> createSolver(const std::string& name, Problem* problem) const;
    std::vector<std::string> availableSolvers() const;

    // Neighborhood factory
    using NeighborhoodFactory = std::function<std::unique_ptr<INeighborhood>()>;
    void registerNeighborhood(const std::string& name, NeighborhoodFactory factory);
    std::unique_ptr<INeighborhood> createNeighborhood(const std::string& name) const;
    std::vector<std::string> availableNeighborhoods() const;

    // Reader factory
    using ReaderFactory = std::function<std::unique_ptr<IReader>()>;
    void registerReader(const std::string& formatName,
                        const std::vector<std::string>& extensions,
                        ReaderFactory factory);
    std::unique_ptr<IReader> createReader(const std::string& formatName) const;
    std::unique_ptr<IReader> createReaderForExtension(const std::string& extension) const;
    std::vector<std::string> availableReaders() const;

    // Testing support
    void clear();
};

} // namespace routing
```

## Plugin Types

### Solver Plugins

Solvers implement the `ISolver` interface:

```cpp
namespace routing {

class ISolver {
public:
    virtual ~ISolver() = default;

    virtual std::string name() const = 0;
    virtual std::string description() const { return ""; }

    virtual void setProblem(Problem* problem) = 0;
    virtual Problem* getProblem() const = 0;

    virtual void setConfiguration(Configuration* config) = 0;
    virtual void setDefaultConfiguration() = 0;

    virtual bool solve(double timeout = 3600) = 0;

    virtual models::Solution* getSolution() const = 0;
    virtual double getObjectiveValue() const = 0;
    virtual bool isOptimal() const { return false; }
    virtual std::string getStats() const { return ""; }
};

} // namespace routing
```

**Available Solvers:**

| Name | Aliases | Description |
|------|---------|-------------|
| `mip` | `cplex` | Exact MIP solver (CPLEX) |
| `genetic` | `ga` | Genetic Algorithm |
| `memetic` | `ma` | Memetic Algorithm |
| `local_search` | `ls` | Local Search |
| `vns` | - | Variable Neighborhood Search |
| `pso` | `particle_swarm` | Particle Swarm Optimization |

**Example Plugin:**

```cpp
// plugins/solvers/MySolverPlugin/MySolverPlugin.hpp
#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/ISolver.hpp"

namespace routing::plugins {

class MySolverWrapper : public ISolver {
public:
    explicit MySolverWrapper(Problem* problem) : problem_(problem) {}

    std::string name() const override { return "my_solver"; }
    void setProblem(Problem* p) override { problem_ = p; }
    Problem* getProblem() const override { return problem_; }
    void setConfiguration(Configuration* c) override { config_ = c; }
    void setDefaultConfiguration() override { /* ... */ }

    bool solve(double timeout) override {
        // Implement solving logic
        return true;
    }

    models::Solution* getSolution() const override { return solution_; }
    double getObjectiveValue() const override {
        return solution_ ? solution_->getCost() : 0.0;
    }

private:
    Problem* problem_;
    Configuration* config_ = nullptr;
    models::Solution* solution_ = nullptr;
};

class MySolverPlugin : public IPlugin {
public:
    std::string name() const override { return "MySolverPlugin"; }
    PluginType type() const override { return PluginType::Solver; }

    void initialize(PluginRegistry& registry) override {
        registry.registerSolver("my_solver",
            [](Problem* problem) -> std::unique_ptr<ISolver> {
                return std::make_unique<MySolverWrapper>(problem);
            });
    }
};

} // namespace routing::plugins
```

### Neighborhood Plugins

Neighborhoods implement the `INeighborhood` interface:

```cpp
namespace routing {

struct NeighborhoodMove {
    std::string type;
    double delta = 0.0;
    std::vector<int> data;
    bool isImproving() const { return delta < 0.0; }
};

class INeighborhood {
public:
    virtual ~INeighborhood() = default;

    virtual std::string name() const = 0;
    virtual std::string description() const { return ""; }

    // Find best move
    virtual std::optional<NeighborhoodMove> explore(models::Solution& solution) = 0;

    // Apply a move
    virtual void apply(models::Solution& solution, const NeighborhoodMove& move) = 0;

    // Convenience: explore and apply if improving
    virtual bool improve(models::Solution& solution);
};

} // namespace routing
```

**Available Neighborhoods:**

| Name | Description |
|------|-------------|
| `two_opt` | 2-opt edge exchange |
| `idch` | Iterated Destruction-Construction |

### Reader Plugins

Readers implement the `IReader` interface:

```cpp
namespace routing {

class IReader {
public:
    virtual ~IReader() = default;

    virtual std::string formatName() const = 0;
    virtual std::vector<std::string> supportedExtensions() const = 0;
    virtual bool canRead(const std::string& filepath) const = 0;
    virtual Problem* readFile(const std::string& filepath) = 0;
    virtual std::string detectedProblemType() const { return "unknown"; }
};

} // namespace routing
```

**Available Readers:**

| Name | Extensions | Description |
|------|------------|-------------|
| `solomon` | `.txt`, `.sol` | Solomon CVRPTW format |
| `tsplib` | `.vrp`, `.tsp` | TSPLIB/CVRPLIB format |

**Usage:**

```cpp
// Read by format name
auto reader = registry.createReader("solomon");
auto* problem = reader->readFile("R101.txt");

// Auto-detect by extension
auto reader = registry.createReaderForExtension(".vrp");
auto* problem = reader->readFile("A-n32-k5.vrp");
```

### Constraint Generator Plugins

See [Composable API Documentation](composable.md) for constraint generators.

## Creating a New Plugin

### Step 1: Create Plugin Header

```cpp
// plugins/my_category/MyPlugin/MyPlugin.hpp
#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"

namespace routing::plugins {

class MyPlugin : public IPlugin {
public:
    std::string name() const override { return "MyPlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "My custom plugin";
    }
    PluginType type() const override { return PluginType::Solver; }

    // Optional: declare dependencies
    std::vector<std::string> dependencies() const override {
        return {"RoutingPlugin"};  // Initialized after RoutingPlugin
    }

    void initialize(PluginRegistry& registry) override {
        // Register components here
    }

    void shutdown() override {
        // Cleanup if needed
    }
};

} // namespace routing::plugins
```

### Step 2: Register in PluginBundle

Edit `plugins/PluginBundle.cpp`:

```cpp
#include "plugins/my_category/MyPlugin/MyPlugin.hpp"

void registerCorePlugins() {
    auto& registry = PluginRegistry::instance();

    // ... existing plugins ...

    registry.registerPlugin(std::make_unique<MyPlugin>());
}
```

And add the include in `plugins/PluginBundle.hpp`:

```cpp
#include "plugins/my_category/MyPlugin/MyPlugin.hpp"
```

### Step 3: Build and Test

```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Plugin Lifecycle

1. registerCorePlugins()
    - Calls registry.registerPlugin() for each plugin
    - Stores plugins in registry
2. registry.initializeAll()
    - Resolves dependencies (topological sort)
    - For each plugin in order:
        - plugin->initialize(registry)
        - Plugin registers its components
    - Sets initialized_ = true
3. [Application uses registered components]
4. registry.shutdownAll()
    - For each plugin in reverse order:
        - plugin->shutdown()

## Dependency Management

Plugins can declare dependencies on other plugins:

```cpp
std::vector<std::string> dependencies() const override {
    return {"ComposableCorePlugin", "RoutingPlugin"};
}
```

The registry uses topological sort to ensure:
- Dependencies are initialized before dependents
- Shutdown happens in reverse order
- Circular dependencies throw an exception

## Querying Available Components

```cpp
auto& registry = routing::PluginRegistry::instance();

// List all available solvers
for (const auto& name : registry.availableSolvers()) {
    std::cout << "Solver: " << name << std::endl;
}

// List all available neighborhoods
for (const auto& name : registry.availableNeighborhoods()) {
    std::cout << "Neighborhood: " << name << std::endl;
}

// List all available readers
for (const auto& name : registry.availableReaders()) {
    std::cout << "Reader: " << name << std::endl;
}

// List all available generators
for (const auto& name : registry.availableGenerators()) {
    std::cout << "Generator: " << name << std::endl;
}
```

## Core Plugins

### Attribute Plugins

| Plugin | Type | Registers |
|--------|------|-----------|
| `ComposableCorePlugin` | Attribute | Core composable infrastructure |
| `RoutingPlugin` | ConstraintGenerator | `RoutingConstraintGenerator` |
| `CapacityPlugin` | Attribute | `CapacityConstraintGenerator` |
| `TimeWindowPlugin` | Attribute | `TimeWindowConstraintGenerator` |
| `ProfitPlugin` | Attribute | `ProfitObjectiveGenerator` |
| `PickupDeliveryPlugin` | Attribute | `PickupDeliveryConstraintGenerator` |
| `SyncPlugin` | Attribute | `SyncConstraintGenerator` |

### Solver Plugins

| Plugin | Registers |
|--------|-----------|
| `GASolverPlugin` | `genetic`, `ga` |
| `LSSolverPlugin` | `local_search`, `ls` |
| `MASolverPlugin` | `memetic`, `ma` |
| `MIPSolverPlugin` | `mip`, `cplex` |
| `PSOSolverPlugin` | `pso`, `particle_swarm` |
| `VNSSolverPlugin` | `vns` |

### Neighborhood Plugins

| Plugin | Registers |
|--------|-----------|
| `TwoOptPlugin` | `two_opt` |
| `IDCHPlugin` | `idch` |

### Reader Plugins

| Plugin | Registers | Extensions |
|--------|-----------|------------|
| `SolomonReaderPlugin` | `solomon` | `.txt`, `.sol` |
| `TSPLIBReaderPlugin` | `tsplib` | `.vrp`, `.tsp` |

## Testing Plugins

Example test using Google Test:

```cpp
#include <gtest/gtest.h>
#include <plugins/PluginBundle.hpp>
#include <core/PluginRegistry.hpp>

class PluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        routing::plugins::registerCorePlugins();
        registry_ = &routing::PluginRegistry::instance();
        registry_->initializeAll();
    }

    void TearDown() override {
        registry_->shutdownAll();
        registry_->clear();
    }

    routing::PluginRegistry* registry_;
};

TEST_F(PluginTest, SolversRegistered) {
    auto solvers = registry_->availableSolvers();
    EXPECT_NE(std::find(solvers.begin(), solvers.end(), "mip"), solvers.end());
    EXPECT_NE(std::find(solvers.begin(), solvers.end(), "vns"), solvers.end());
}

TEST_F(PluginTest, CreateSolver) {
    // Need a problem to create solver
    routing::ComposableProblem problem;
    auto solver = registry_->createSolver("mip", &problem);
    EXPECT_NE(solver, nullptr);
    EXPECT_EQ(solver->name(), "mip");
}
```

## Best Practices

1. **One Plugin Per Component Type**: Keep plugins focused
2. **Declare Dependencies**: Use `dependencies()` if order matters
3. **Check Before Registering**: Use `hasGenerator()`, etc. to avoid duplicates
4. **Use Factory Pattern**: Register factories, not instances
5. **Clean Shutdown**: Implement `shutdown()` if you allocate resources
6. **Version Your Plugins**: Use `version()` for compatibility tracking

## Directory Structure

```
plugins/
+-- CMakeLists.txt
+-- PluginBundle.hpp      # Declares registerCorePlugins()
+-- PluginBundle.cpp      # Implements plugin registration
+-- attributes/
|   +-- ComposableCorePlugin/
|   +-- RoutingPlugin/
|   +-- CapacityPlugin/
|   +-- TimeWindowPlugin/
|   +-- ProfitPlugin/
|   +-- PickupDeliveryPlugin/
|   +-- SyncPlugin/
+-- solvers/
|   +-- GASolverPlugin/
|   +-- LSSolverPlugin/
|   +-- MASolverPlugin/
|   +-- MIPSolverPlugin/
|   +-- PSOSolverPlugin/
|   +-- VNSSolverPlugin/
+-- neighborhoods/
|   +-- TwoOptPlugin/
|   +-- IDCHPlugin/
+-- readers/
    +-- SolomonReaderPlugin/
    +-- TSPLIBReaderPlugin/
```
