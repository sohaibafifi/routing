<div align="center">
  <img src="logo.svg" width="500" alt="Routing" />
</div>

<!-- START badges-list -->
<div align="center">

![](https://img.shields.io/static/v1.svg?label=version&message=-.-.-&color=red&style=flat-square)
![](https://img.shields.io/static/v1.svg?label=release%20date&message=??-??-????&color=red&style=flat-square)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/85e69139d552469fa1c0b0f1b098f60f)](https://app.codacy.com/manual/me_183/routing?utm_source=github.com&utm_medium=referral&utm_content=sohaibafifi/routing&utm_campaign=Badge_Grade_Dashboard)

![](https://img.shields.io/static/v1.svg?label=license&message=TODO&color=green&style=flat-square)
![](https://img.shields.io/static/v1.svg?label=C%2b%2b&message=TODO&color=informational&style=flat-square)
![](https://img.shields.io/static/v1.svg?label=Cplex&message=12.10&color=informational&style=flat-square)

</div>
<!-- END badges-list -->

<div align="center">
<h4>A library to solve vehicle routing problems</h4>
</div>

<div align="center">
<a href="LICENSE">License</a> •
<a href="CONTRIBUTING.md">Contributing</a> •
<a href="CHANGELOG.md">Change log</a>

<a href="doc/options.md">Solver options</a> •
<a href="doc/usage.md">Create a custom VRP solver</a> •
<a href="doc/faq.md">FAQ</a>
</div>

## About

TODO: General description of the project

## Installation

Using this library requires first the installation of the following programs:

* [CMake](https://cmake.org/download/) ;
* [CPLEX](https://www.ibm.com/fr-fr/products/ilog-cplex-optimization-studio).

## First steps

To use the library, you have first to fetch the dependencies and prepare the
build files of the project:

```bash
git submodule update --init --recursive
mkdir -p build && cd build && cmake .. && cd ..
```

Once done, the project is compiled and tested using the following commands:

* `make` to compile the code of the library ;

* `make test` to compile the tests of the library.

To run the provided example, a solver for the simple VRP problem, using
a specific instance from data, run the following command:

* `./build/main -i data/CVRP/toy.vrp`

The solver has many options aside from _"-i"_ (input file).
Their description can be read in the [Options](doc/options.md) part of the
documentation.

## Create your own solver

The library supports the creation of solvers for any custom VRP problem.
Please refer to the [Usage documentation](doc/usage.md) to create your
own solver.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## Supported versions

The project has been developped and checked in a MacOS environment.
It relies on the following programs :

| Dependency | Version |
|------------|---------|
| gcc & g++  |         |
| CMake      |         |
| CPLEX      | 12.10   |

## License

You are allowed to retrieve this project for research purposes as a member of a non-commercial and academic institution.

## Acknowledgements

Registered contributors are:

* Sohaib LAFIFI - _<sohaib.lafifi@univ-artois.fr>_ - designer,developer
* Yoann KUBERA - _<yoann.kubera@univ-artois.fr>_ - documentation
