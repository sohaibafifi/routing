// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "plugins/attributes/ComposableCorePlugin/ComposableCorePlugin.hpp"
#include "plugins/attributes/RoutingPlugin/RoutingPlugin.hpp"
#include "plugins/attributes/CapacityPlugin/CapacityPlugin.hpp"
#include "plugins/attributes/TimeWindowPlugin/TimeWindowPlugin.hpp"
#include "plugins/attributes/ProfitPlugin/ProfitPlugin.hpp"
#include "plugins/attributes/PickupDeliveryPlugin/PickupDeliveryPlugin.hpp"
#include "plugins/attributes/SyncPlugin/SyncPlugin.hpp"

#include "plugins/solvers/GASolverPlugin/GASolverPlugin.hpp"
#include "plugins/solvers/LSSolverPlugin/LSSolverPlugin.hpp"
#include "plugins/solvers/MASolverPlugin/MASolverPlugin.hpp"
#include "plugins/solvers/MIPSolverPlugin/MIPSolverPlugin.hpp"
#include "plugins/solvers/PSOSolverPlugin/PSOSolverPlugin.hpp"
#include "plugins/solvers/VNSSolverPlugin/VNSSolverPlugin.hpp"
#if defined(CPLEX_FOUND) || defined(ORTOOLS_FOUND)
#include "plugins/solvers/CPSolverPlugin/CPSolverPlugin.hpp"
#endif

#include "plugins/neighborhoods/TwoOptPlugin/TwoOptPlugin.hpp"
#include "plugins/neighborhoods/IDCHPlugin/IDCHPlugin.hpp"

#include "plugins/readers/SolomonReaderPlugin/SolomonReaderPlugin.hpp"
#include "plugins/readers/TSPLIBReaderPlugin/TSPLIBReaderPlugin.hpp"

namespace routing::plugins {
    void registerCorePlugins();
} // namespace routing::plugins
