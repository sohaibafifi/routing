<div align="center">
  <h1>Create your own solver</h1>
</div>
<div align="center">
  <a href="../README.md">⮌ Parent</a> •
  <a href="#presentation">Presentation</a> •
  <a href="#files-and-directories">Files and directories</a> •
  <a href="#instances">Instances</a> •
  <a href="#reader">Reader</a> •
  <a href="#problem">Problem</a> •
  <a href="#main">Main</a> •
</div>

## Presentation

In this library, a VRP solver is made of the following parts:

* a __Reader__ file that can translate an input file into a VRP problem formulation,
  and especially the introduction of custom data (time windows, vehicle capacity,
  _etc._) ;

* a __Problem__ file providing a model to the custom VRP problem, relying on
  custom implementation of the core features of a VRP problem (client, vehicle,
  tour, solution, _etc._);

* a set of __instances__ on which the solver can be run (files in `data/` provided
  as inputs of the solver) ;

* a __main file__, used to run the solver on a specific problem.

## Files and directories

The core files of the problem (related to the reader and the problem) have to be
managed in a sub-directory of `problems/`, named after your flavor of the VRP
problem. It has to contain :

* a `src/` directory, where the source files have to be put ;

* a `CMakeLists.txt` file describing how to compile them.

Finally, the main file that will run the instances has to be put in the `src/`
directory at the root of this project.

## Instances

TODO : Write a documentation

## Reader

TODO : Write a documentation

## Problem

TODO : Write a documentation

## Main

The main file of a VRP solver always follow the same structure.
Therefore, creating a new one usually consists in copying an existing one, and
changing the Reader, Problem and Solver classes in it.

See the [../src/main.cpp](../src/main.cpp) example file for the regular VRP
problem.
