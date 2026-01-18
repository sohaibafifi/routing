# Installation

This page provides detailed installation instructions for all platforms and configurations.

---

## System Requirements

### Minimum Requirements

| Component | Python | C++ |
|-----------|--------|-----|
| **Python** | 3.9+ | Not required |
| **Compiler** | C++20 support | GCC 10+, Clang 12+, MSVC 2019+ |
| **CMake** | 3.16+ (for building) | 3.16+ |
| **Git** | Required | Required |

### Supported Platforms

| Platform | Python | C++ | Notes |
|----------|--------|-----|-------|
| **Linux** (Ubuntu 20.04+) | Full support | Full support | Recommended |
| **macOS** (12+) | Full support | Full support | Homebrew recommended |
| **Windows** (10+) | Full support | Full support | Visual Studio 2019+ |

---

## Quick Install (Python)

For most users, this is the recommended approach:

```bash
# Clone the repository
git clone https://github.com/sohaibafifi/rihla.git
cd routing

# Install Python package
cd python
pip install -e .
```

Verify the installation:

```python
import routing
routing.init()
print("Available solvers:", routing.list_solvers())
```

---

## Platform-Specific Instructions

::::{tab-set}

:::{tab-item} Linux (Ubuntu/Debian)
:sync: linux

### Install Prerequisites

```bash
# Update package lists
sudo apt update

# Install build tools
sudo apt install -y build-essential cmake git python3-dev python3-pip

# Install optional: HiGHS (open-source MIP solver)
sudo apt install -y coinor-libhighs-dev

# Install optional: OR-Tools
pip install ortools
```

### Build from Source

```bash
# Clone with submodules
git clone https://github.com/sohaibafifi/rihla.git
cd routing
git submodule update --init --recursive

# Configure
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DROUTING_BUILD_EXAMPLES=ON \
    -DBUILD_TESTING=ON

# Build (use all cores)
cmake --build build --parallel $(nproc)

# Run tests
ctest --test-dir build --output-on-failure
```

### Python Package

```bash
cd python
pip install -e .
```

:::

:::{tab-item} macOS
:sync: macos

### Install Prerequisites

```bash
# Install Homebrew if not present
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install build tools
brew install cmake git python@3.11

# Install optional: HiGHS
brew install highs

# Install optional: OR-Tools
brew install or-tools
```

### Build from Source

```bash
# Clone with submodules
git clone https://github.com/sohaibafifi/rihla.git
cd routing
git submodule update --init --recursive

# Configure
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DROUTING_BUILD_EXAMPLES=ON

# Build
cmake --build build --parallel $(sysctl -n hw.ncpu)

# Run tests
ctest --test-dir build --output-on-failure
```

### Python Package

```bash
cd python
pip install -e .
```

:::

:::{tab-item} Windows
:sync: windows

### Install Prerequisites

1. **Visual Studio 2019+** with "Desktop development with C++" workload
2. **CMake** (3.16+): [Download](https://cmake.org/download/)
3. **Git**: [Download](https://git-scm.com/download/win)
4. **Python 3.9+**: [Download](https://www.python.org/downloads/)

### Build from Source (PowerShell)

```powershell
# Clone with submodules
git clone https://github.com/sohaibafifi/rihla.git
cd routing
git submodule update --init --recursive

# Configure (Visual Studio 2019)
cmake -S . -B build -G "Visual Studio 16 2019" -A x64

# Build Release
cmake --build build --config Release --parallel

# Run tests
ctest --test-dir build -C Release --output-on-failure
```

### Python Package

```powershell
cd python
pip install -e .
```

:::

::::

---

## CMake Build Options

The following CMake options control the build:

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | Build type: `Debug`, `Release`, `RelWithDebInfo` |
| `ROUTING_BUILD_EXAMPLES` | `ON` | Build example applications |
| `BUILD_TESTING` | `ON` | Build test suite |
| `ROUTING_BUILD_CPOPTIMIZER` | `ON` | Build CPLEX CP Optimizer backend |
| `ROUTING_BUILD_GRB` | `OFF` | Build Gurobi support |
| `ROUTING_BUILD_ORTOOLS` | `ON` | Build OR-Tools CP-SAT backend |
| `ROUTING_BUILD_HIGHS` | `ON` | Build HiGHS MIP backend |

### Example: Minimal Build

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DROUTING_BUILD_EXAMPLES=OFF \
    -DBUILD_TESTING=OFF
```

### Example: Full Build with All Solvers

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DROUTING_BUILD_CPOPTIMIZER=ON \
    -DROUTING_BUILD_GRB=ON \
    -DROUTING_BUILD_ORTOOLS=ON \
    -DROUTING_BUILD_HIGHS=ON \
    -DCPLEX_ROOT=/opt/ibm/ILOG/CPLEX_Studio221 \
    -DGUROBI_HOME=/opt/gurobi1000
```

---

## Optional Solver Backends

The library supports multiple solver backends. Install the ones you need:

### HiGHS (Open Source MIP)

HiGHS is a high-performance open-source linear and mixed-integer programming solver.

::::{tab-set}

:::{tab-item} Linux
```bash
sudo apt install coinor-libhighs-dev
# Or from source:
git clone https://github.com/ERGO-Code/HiGHS.git
cd HiGHS && cmake -S . -B build && cmake --build build --parallel
sudo cmake --install build
```
:::

:::{tab-item} macOS
```bash
brew install highs
```
:::

:::{tab-item} Windows
Download pre-built binaries from [HiGHS releases](https://github.com/ERGO-Code/HiGHS/releases).
:::

::::

**CMake Detection:**
```bash
cmake -S . -B build -DHIGHS_ROOT=/path/to/highs
```

---

### OR-Tools (Open Source CP-SAT)

Google OR-Tools provides a powerful constraint programming solver.

::::{tab-set}

:::{tab-item} Linux
```bash
# Via pip (Python only)
pip install ortools

# System-wide (C++)
wget https://github.com/google/or-tools/releases/download/v9.8/or-tools_amd64_ubuntu-22.04_cpp_v9.8.3296.tar.gz
tar xzf or-tools_*.tar.gz
sudo mv or-tools_*/ /opt/or-tools
```
:::

:::{tab-item} macOS
```bash
brew install or-tools
# Or via pip
pip install ortools
```
:::

:::{tab-item} Windows
Download from [OR-Tools releases](https://github.com/google/or-tools/releases).
:::

::::

**CMake Detection:**
```bash
cmake -S . -B build -DORTOOLS_ROOT=/opt/or-tools
```

---

### XCSP3 (Open Standard)

[XCSP3](https://www.xcsp.org/) is an XML-based format for constraint satisfaction and optimization problems. The library includes a built-in XCSP3 backend that can:

- **Export** problems to XCSP3 format
- **Interface** with any XCSP3-compatible solver

:::{admonition} No Installation Required
:class: tip

XCSP3 export is built into the library - no additional dependencies needed! The exported model is then solved using ACE solver.
:::

**Compatible External Solvers:**

| Solver | Language | Link |
|--------|----------|------|
| ACE | Java | [GitHub](https://github.com/xcsp3team/ACE) |
| Choco | Java | [choco-solver.org](https://choco-solver.org/) |
| PicatSAT | Picat | [picat-lang.org](http://picat-lang.org/) |
| AbsCon | Java | [XCSP.org](https://www.xcsp.org/) |

**Usage:**
```python
solver = routing.create_solver("cp/ace", problem)
solver.export_model("problem.xml")  # Export to file
```

Learn more at [xcsp.org](https://www.xcsp.org/).

---

### IBM CPLEX (Commercial)

CPLEX provides both MIP and CP Optimizer solvers. Requires a license.

1. Download from [IBM Academic Initiative](https://www.ibm.com/academic) (free for academics)
2. Install to default location or custom path

**CMake Detection:**
```bash
# Default installation
cmake -S . -B build

# Custom path
cmake -S . -B build -DCPLEX_ROOT=/opt/ibm/ILOG/CPLEX_Studio221
```

:::{note}
Set `CPLEX_ROOT` to the CPLEX Studio installation directory containing `cplex/`, `cpoptimizer/`, and `concert/` subdirectories.
:::

---

### Gurobi (Commercial)

Gurobi is a high-performance commercial MIP solver.

1. Download from [Gurobi](https://www.gurobi.com/downloads/)
2. Obtain a license (free for academics)
3. Install and set environment variables

**CMake Detection:**
```bash
cmake -S . -B build \
    -DROUTING_BUILD_GRB=ON \
    -DGUROBI_HOME=/opt/gurobi1000/linux64
```

---

## Verifying Installation

### Python

```python
import routing

# Initialize
routing.init()

# Check available solvers
solvers = routing.list_solvers()
print("Available solvers:", solvers)

# Expected output includes some of:
# - 'ga', 'ma', 'vns', 'pso', 'ls' (metaheuristics, always available)
# - 'mip/cplex', 'cp/cpo' (if CPLEX installed)
# - 'mip/highs' (if HiGHS installed)
# - 'cp/ortools' (if OR-Tools installed)
```

### C++

```bash
# Run the test suite
ctest --test-dir build --output-on-failure

# Run an example
./build/examples/composable/example_composable
```

---

## Development Installation

For contributors working on the library:

```bash
# Clone with all history
git clone https://github.com/sohaibafifi/rihla.git
cd routing
git submodule update --init --recursive

# Debug build with tests
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build --parallel

# Run tests
ctest --test-dir build --output-on-failure

# Install Python in development mode
cd python
pip install -e ".[dev]"
```

---

## Docker

A Docker image is available for quick setup:

```dockerfile
# Dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git python3-dev python3-pip \
    coinor-libhighs-dev

WORKDIR /app
COPY . .

RUN git submodule update --init --recursive && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel

RUN cd python && pip install -e .

CMD ["python3", "-c", "import routing; routing.init(); print(routing.list_solvers())"]
```

Build and run:

```bash
docker build -t routing .
docker run -it routing
```

---

## Troubleshooting

### Common Issues

#### `ImportError: routing._routing_core not found`

The C++ extension wasn't built. Rebuild:

```bash
cd python
pip install -e . --no-build-isolation
```

#### `CMake Error: Could not find CPLEX`

CPLEX is not installed or not in the expected location:

```bash
cmake -S . -B build -DCPLEX_ROOT=/path/to/cplex/studio
# Or disable CPLEX:
cmake -S . -B build -DROUTING_BUILD_CPOPTIMIZER=OFF
```

#### `Solver 'mip/highs' not available`

HiGHS was not found during build. Install HiGHS and rebuild:

```bash
# Install HiGHS
brew install highs  # macOS
# or
sudo apt install coinor-libhighs-dev  # Linux

# Rebuild
cmake -S . -B build && cmake --build build
```

#### `error: 'filesystem' is not a namespace`

Your compiler doesn't fully support C++20. Upgrade:

```bash
# Ubuntu
sudo apt install g++-11
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-11

# macOS
brew install llvm
cmake -S . -B build -DCMAKE_CXX_COMPILER=$(brew --prefix llvm)/bin/clang++
```

#### Tests failing with `CPLEX not found`

Some tests require CPLEX. Run only non-CPLEX tests:

```bash
ctest --test-dir build -E "cplex" --output-on-failure
```

---

## Next Steps

- **[Getting Started](getting-started.md)**: Write your first routing problem
- **[Concepts](concepts.md)**: Learn about the composable attribute system
- **[Solvers](solvers.md)**: Choose and configure solvers
