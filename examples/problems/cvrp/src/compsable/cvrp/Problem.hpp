// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <plugins/attributes/ComposableCorePlugin/Problem.hpp>
#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>

namespace compsable::cvrp {

class Problem : public routing::Problem {
public:
    Problem() {
        enableAttributes<
            routing::attributes::GeoNode,
            routing::attributes::Consumer,
            routing::attributes::Stock>();
    }

    using routing::Problem::addClient;
    using routing::Problem::addDepot;
    using routing::Problem::addVehicle;

    routing::Client* addClient(unsigned id,
                               routing::Duration x,
                               routing::Duration y,
                               routing::Demand demand) {
        auto* client = routing::Problem::addClient(id);
        client->addAttribute<routing::attributes::GeoNode>(x, y);
        client->addAttribute<routing::attributes::Consumer>(demand);
        return client;
    }

    routing::Depot* addDepot(unsigned id,
                             routing::Duration x,
                             routing::Duration y) {
        auto* depot = routing::Problem::addDepot(id);
        depot->addAttribute<routing::attributes::GeoNode>(x, y);
        return depot;
    }

    routing::Vehicle* addVehicle(unsigned id, routing::Capacity capacity) {
        auto* vehicle = routing::Problem::addVehicle(id);
        vehicle->addAttribute<routing::attributes::Stock>(capacity);
        return vehicle;
    }
};

} // namespace compsable::cvrp
