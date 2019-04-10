//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_VEHICLE_H
#define HYBRID_VEHICLE_H

#include "../../cvrp/models/Vehicle.hpp"

namespace CVRPTW{
    struct Vehicle
            :
                    public CVRP::Vehicle
    {
        Vehicle(unsigned id, routing::Capacity capacity)
                :  CVRP::Vehicle(id, capacity)
        {
        }
    };
}

#endif //HYBRID_VEHICLE_H
