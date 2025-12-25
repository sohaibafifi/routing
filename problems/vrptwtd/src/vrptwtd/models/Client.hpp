// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrptw/models/Client.hpp>
#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/TimeWindowPlugin/Service.hpp>
#include <plugins/attributes/TimeWindowPlugin/Appointment.hpp>
#include <plugins/attributes/TimeWindowPlugin/Rendezvous.hpp>
#include <core/interfaces/IAttribute.hpp>
#include "../attributes/Synced.hpp"

namespace vrptwtd {
    namespace models {
        struct Client : public cvrptw::models::Client,
                        public vrptwtd::attributes::Synced {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::Demand &demand,
                   const routing::Duration &p_service, const routing::TW &timewindow) :
                    cvrptw::models::Client(id, x, y, demand, p_service, timewindow),
                   attributes::Synced(std::vector<attributes::Synced*>(), std::vector<routing::Duration>()) {
            }

            Client(cvrptw::models::Client *client) :
                    cvrptw::models::Client(*client),
                   attributes::Synced(std::vector<attributes::Synced*>(), std::vector<routing::Duration>()) {
                attributes::Synced::setID(client->getID());
            }
        };
    }
}