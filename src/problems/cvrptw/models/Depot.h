//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_DEPOT_H
#define HYBRID_DEPOT_H

#include "../../../data/attributes/GeoNode.hpp"
#include "../../../data/attributes/Rendezvous.hpp"
#include "../../cvrp/models/Depot.hpp"

namespace CVRPTW{
    struct Depot
            :
                    public CVRP::Depot,
                    public routing::attributes::Rendezvous
    {
        Depot(unsigned id,
              const routing::Duration &x,
              const routing::Duration &y,
              const routing::TW &timewindow)
                :
                CVRP::Depot(id, x, y),
                routing::attributes::Rendezvous(timewindow)
        {
        }
    };
}


#endif //HYBRID_DEPOT_H
