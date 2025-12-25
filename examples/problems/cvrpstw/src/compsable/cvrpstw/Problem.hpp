// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <compsable/cvrptw/Problem.hpp>
#include <plugins/attributes/TimeWindowPlugin/SoftTimeWindows.hpp>

namespace compsable::cvrpstw {

class Problem : public compsable::cvrptw::Problem {
public:
    Problem() : compsable::cvrptw::Problem() {
        enableAttribute<routing::attributes::SoftTimeWindows>();
    }

    using compsable::cvrptw::Problem::addClient;
    using compsable::cvrptw::Problem::addDepot;
    using compsable::cvrptw::Problem::addVehicle;

    routing::Depot* addDepot(unsigned id,
                             routing::Duration x,
                             routing::Duration y,
                             routing::Duration twOpen,
                             routing::Duration twClose,
                             double waitPenalty,
                             double delayPenalty) {
        auto* depot = compsable::cvrptw::Problem::addDepot(id, x, y, twOpen, twClose);
        depot->addAttribute<routing::attributes::SoftTimeWindows>(waitPenalty, delayPenalty);
        return depot;
    }
};

} // namespace compsable::cvrpstw
