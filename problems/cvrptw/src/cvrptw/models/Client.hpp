// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrp/models/Client.hpp>
#include <core/data/attributes/Consumer.hpp>
#include <core/data/attributes/Service.hpp>
#include <core/data/attributes/Appointment.hpp>
#include <core/data/attributes/Rendezvous.hpp>
#include <core/data/attributes.hpp>

namespace cvrptw {
    namespace models {
        struct Client : public cvrp::models::Client,
                        public routing::attributes::ServiceQuery, public routing::attributes::Rendezvous {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::Demand &demand,
                   const routing::Duration &p_service, const routing::TW &timewindow) :
                    cvrp::models::Client(id, x, y, demand),
                    routing::attributes::ServiceQuery(p_service),
                    routing::attributes::Rendezvous(timewindow) {
            }
        };

        struct Visit :
                public routing::attributes::Service,
                public routing::attributes::Appointment {
            Visit(Client *p_client,
                  const routing::Duration &p_start,
                  const routing::Duration &p_maxshift,
                  const routing::Duration &p_wait) :
                    routing::attributes::Service(p_start),
                    routing::attributes::Appointment(p_maxshift, p_wait),
                    client(p_client) {}

            Client *client;
        };

    }
}