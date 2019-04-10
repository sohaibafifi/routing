//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_CLIENT_H
#define HYBRID_CLIENT_H

#include "../../../data/attributes/ServiceQuery.hpp"
#include "../../../data/attributes/Consumer.hpp"
#include "../../../data/attributes/Rendezvous.hpp"
#include "../../cvrp/models/Client.hpp"

namespace CVRPTW {
    struct Client
            :
                    public CVRP::Client,
                    public routing::attributes::Rendezvous,
                    public routing::attributes::ServiceQuery {
        Client(unsigned id,
               const routing::Demand &demand,
               const routing::Duration &service,
               const routing::TW &timewindow,
               const routing::Duration &x,
               const routing::Duration &y)
                :
                CVRP::Client(id, demand, x, y),
                routing::attributes::Rendezvous(timewindow),
                routing::attributes::ServiceQuery(service) {
        }

    };

}

#endif //HYBRID_CLIENT_H
