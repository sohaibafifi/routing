//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_VISIT_H
#define HYBRID_VISIT_H

#include "../../../data/attributes/Appointment.hpp"
#include "../../../data/attributes/Service.hpp"
#include "Client.hpp"
namespace CVRPTW{
    struct Visit:
            public routing::attributes::Service,
            public routing::attributes::Appointment
    {
        Visit(Client * p_client,
              const routing::Duration & p_start,
              const routing::Duration& p_maxshift,
              const routing::Duration& p_wait):
                routing::attributes::Service(p_start),
                routing::attributes::Appointment(p_maxshift, p_wait),
                client(p_client)
        {}
        Client * client;
    };

}

#endif //HYBRID_VISIT_H
