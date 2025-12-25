// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <examples/problems/cvrp/Problem.hpp>
#include <plugins/attributes/ProfitPlugin/Profiter.hpp>

namespace composable::top {

class Problem : public composable::cvrp::Problem {
public:
    Problem() : composable::cvrp::Problem() {
        enableAttribute<routing::attributes::Profiter>();
    }

    using composable::cvrp::Problem::addClient;
    using composable::cvrp::Problem::addDepot;
    using composable::cvrp::Problem::addVehicle;

    routing::Client* addClient(unsigned id,
                               routing::Duration x,
                               routing::Duration y,
                               routing::Demand demand,
                               routing::Profit profit) {
        auto* client = routing::Problem::addClient(id);
        client->addAttribute<routing::attributes::GeoNode>(x, y);
        client->addAttribute<routing::attributes::Consumer>(demand);
        client->addAttribute<routing::attributes::Profiter>(profit);
        return client;
    }

    routing::Client* addClient(unsigned id,
                               routing::Duration x,
                               routing::Duration y,
                               routing::Demand demand) {
        return addClient(id, x, y, demand, demand);
    }
};

} // namespace composable::top
