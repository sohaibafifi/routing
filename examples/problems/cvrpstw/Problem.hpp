// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <examples/problems/cvrptw/Problem.hpp>
#include <plugins/attributes/TimeWindowPlugin/SoftTimeWindows.hpp>

namespace composable::cvrpstw {

class Problem : public composable::cvrptw::Problem {
public:
    Problem() : composable::cvrptw::Problem() {
        enableAttribute<routing::attributes::SoftTimeWindows>();
    }

    using composable::cvrptw::Problem::addClient;
    using composable::cvrptw::Problem::addDepot;
    using composable::cvrptw::Problem::addVehicle;

    routing::Depot* addDepot(unsigned id,
                             routing::Duration x,
                             routing::Duration y,
                             routing::Duration twOpen,
                             routing::Duration twClose,
                             double waitPenalty,
                             double delayPenalty) {
        auto* depot = composable::cvrptw::Problem::addDepot(id, x, y, twOpen, twClose);
        depot->addAttribute<routing::attributes::SoftTimeWindows>(waitPenalty, delayPenalty);
        return depot;
    }
};

} // namespace composable::cvrpstw
