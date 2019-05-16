#ifndef HYBRID_CVRPTW_H
#define HYBRID_CVRPTW_H

#include "models/Client.hpp"
#include "models/Depot.hpp"
#include "models/Vehicle.hpp"
#include "models/Solution.hpp"
#include "models/Tour.hpp"
#include "models/Visit.hpp"
#include "models/Solution.hpp"
#include "Reader.hpp"
#include "routines/operators/RandomDestructor.hpp"
#include "routines/operators/SequentialDestructor.hpp"
#include "routines/operators/Constructor.hpp"
#include "routines/Callbacks/HeuristicCallback.hpp"
#include "../cvrp/cvrp.hpp"
#include "solvers/LSSolver.hpp"
#endif //HYBRID_CVRPTW_H