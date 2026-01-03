# Contributing

Thank you for your interest in contributing to the Routing library! This guide will help you get started.

---

## Development Setup

### Prerequisites

- Git
- CMake 3.16+
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Python 3.9+ (for Python bindings)

### Clone and Build

```bash
# Clone with submodules
git clone https://github.com/sohaibafifi/rihla.git
cd routing
git submodule update --init --recursive

# Debug build with compile commands (for IDE support)
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build build --parallel

# Run tests
ctest --test-dir build --output-on-failure
```

### Python Development

```bash
cd python
pip install -e ".[dev]"  # Install with dev dependencies
```

---

## Code Style

### C++ Guidelines

| Rule | Example |
|------|---------|
| **One class per file** | `MyClass.hpp` contains `class MyClass` |
| **Header guards** | Use `#pragma once` |
| **Namespaces** | Match directory: `problems/cvrp/` uses `namespace cvrp` |
| **Naming** | `PascalCase` for classes, `camelCase` for methods/variables |
| **Ownership** | Prefer references when ownership is not transferred |
| **Include order** | Standard lib, third-party, project headers |

### Example Header

```cpp
#pragma once

#include <vector>
#include <memory>

#include "routing/Problem.hpp"

namespace routing {

class MyConstraintGenerator : public IConstraintGenerator {
public:
    explicit MyConstraintGenerator(Problem* problem);

    std::string name() const override;
    void addVariables(IBackend& backend) override;
    void addConstraints(IBackend& backend) override;

private:
    Problem* problem_;
};

}  // namespace routing
```

### Python Guidelines

- Follow PEP 8
- Use type hints
- Write docstrings for public APIs

---

## Project Structure

```
routing/
├── core/src/core/           # Core library
│   ├── data/                # Problem data structures
│   ├── solvers/             # Solver interfaces
│   └── constraints/         # Constraint generators
├── plugins/                 # Plugin system
│   ├── attributes/          # Attribute plugins
│   ├── solvers/             # Solver plugins
│   └── constraints/         # Constraint plugins
├── problems/                # Legacy problem hierarchies
├── examples/                # Example applications
├── tests/                   # Test suite
├── python/                  # Python bindings
└── docs/                    # Documentation
```

---

## Adding New Features

### Adding a Constraint Generator

1. Create header in `core/src/core/constraints/` or `plugins/constraints/`:

```cpp
#pragma once

#include "core/constraints/IConstraintGenerator.hpp"
#include "core/data/AttributeRegistry.hpp"

namespace routing {

class MyConstraintGenerator : public IConstraintGenerator {
public:
    std::string name() const override {
        return "MyConstraint";
    }

    std::vector<std::string> requiredAttributes() const override {
        return {"MyAttribute"};
    }

    void addVariables(IBackend& backend) override {
        // Create variables
    }

    void addConstraints(IBackend& backend) override {
        // Add constraints
    }
};

// Auto-register with the registry
ROUTING_REGISTER_GENERATOR(MyConstraintGenerator)

}  // namespace routing
```

2. Add to CMakeLists.txt if needed
3. Add tests

### Adding a Solver

1. Inherit from `routing::Solver` in `core/src/core/solvers/`
2. Implement required methods:

```cpp
class MySolver : public Solver {
public:
    explicit MySolver(Problem* problem);

    bool solve(double timeout) override;
    void setDefaultConfiguration() override;
    double getObjectiveValue() const override;
};
```

3. Register in a plugin

### Adding an Attribute

1. Create attribute class using CRTP:

```cpp
struct MyAttribute : public Attribute<MyAttribute> {
    static constexpr const char* name() { return "MyAttribute"; }

    double value;

    explicit MyAttribute(double v) : value(v) {}
};
```

2. Register in attribute registry

---

## Testing

### Running Tests

```bash
# All tests
ctest --test-dir build --output-on-failure

# Specific test
ctest --test-dir build -R "composableApiTest" --output-on-failure

# With verbose output
ctest --test-dir build -V

# Tests matching pattern
ctest --test-dir build -R "cp_" --output-on-failure
```

### Writing Tests

Tests use Google Test. Place them in `tests/`:

```cpp
#include <gtest/gtest.h>
#include "routing/Problem.hpp"

TEST(MyFeatureTest, BasicUsage) {
    routing::Problem problem;
    // ... test code
    EXPECT_EQ(expected, actual);
}
```

---

## Pull Request Process

1. **Fork** the repository
2. **Create a branch** from `develop`:
   ```bash
   git checkout -b feature/my-feature develop
   ```
3. **Make changes** following the code style guidelines
4. **Add tests** for new functionality
5. **Run tests** to ensure nothing is broken
6. **Commit** with clear messages:
   ```
   feat: Add support for soft time windows

   - Add SoftTimeWindow attribute
   - Implement penalty calculation
   - Add tests for soft TW constraints
   ```
7. **Push** and create a Pull Request to `develop`

### Commit Message Format

```
<type>: <description>

[optional body]

[optional footer]
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `refactor`: Code refactoring
- `test`: Adding tests
- `chore`: Maintenance

---

## Documentation

### Building Docs Locally

```bash
# Install dependencies
pip install -r docs/requirements.txt

# Build HTML
cd docs
sphinx-build -b html . _build/html

# Open in browser
open _build/html/index.html  # macOS
xdg-open _build/html/index.html  # Linux
```

### Writing Documentation

- Use Markdown (MyST) for documentation files
- Add docstrings to Python code
- Update relevant docs when adding features

---

## Getting Help

- **Issues**: Report bugs or request features on [GitHub Issues](https://github.com/sohaibafifi/rihla/issues)
- **Discussions**: Ask questions on [GitHub Discussions](https://github.com/sohaibafifi/rihla/discussions)

---

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.
