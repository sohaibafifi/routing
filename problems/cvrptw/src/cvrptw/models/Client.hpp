//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <vrp/models/Client.hpp>
#include <core/data/attributes/Consumer.hpp>
#include <core/data/attributes/Service.hpp>
#include <core/data/attributes/Appointment.hpp>
#include <core/data/attributes/Rendezvous.hpp>
#include <core/data/attributes.hpp>

namespace cvrptw {
    namespace models {
        struct Client : public vrp::models::Client, public routing::attributes::Consumer,
                        public routing::attributes::ServiceQuery, public routing::attributes::Rendezvous {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::Demand &demand,
                   const routing::Duration &p_service, const routing::TW &timewindow) :
                    vrp::models::Client(id, x, y),
                    routing::attributes::Consumer(demand),
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