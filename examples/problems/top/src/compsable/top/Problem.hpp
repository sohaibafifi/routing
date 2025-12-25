// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <compsable/cvrp/Problem.hpp>
#include <plugins/attributes/ProfitPlugin/Profiter.hpp>

namespace compsable::top {

class Problem : public compsable::cvrp::Problem {
public:
    Problem() : compsable::cvrp::Problem() {
        enableAttribute<routing::attributes::Profiter>();
    }

    using compsable::cvrp::Problem::addClient;
    using compsable::cvrp::Problem::addDepot;
    using compsable::cvrp::Problem::addVehicle;

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

} // namespace compsable::top
