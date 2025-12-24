# Routing

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/85e69139d552469fa1c0b0f1b098f60f)](https://app.codacy.com/manual/me_183/routing?utm_source=github.com&utm_medium=referral&utm_content=sohaibafifi/routing&utm_campaign=Badge_Grade_Dashboard)

A library to solve vehicle routing problems


## Compilation and tests

```bash

git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Useful CMake options:
- `-DROUTING_BUILD_EXAMPLES=OFF` (skip examples)
- `-DBUILD_TESTING=OFF` (skip tests)
- `-DROUTING_BUILD_CPOPTIMIZER=OFF` (skip `cpoptimizer`)
- `-DROUTING_BUILD_GRB=ON` (build `grb` if GUROBI is found)

On macOS, the build defaults to your host architecture (e.g. `arm64`). Override with `-DCMAKE_OSX_ARCHITECTURES=...` if needed.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
You are allowed to retrieve this project for research purposes as a member of a non-commercial and academic institution.
