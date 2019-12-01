//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <vrp/models/Depot.hpp>
#include <core/data/attributes/Consumer.hpp>
#include <core/data/attributes/Rendezvous.hpp>
#include <core/data/attributes.hpp>

namespace cvrptw {
    namespace models {
        struct Depot : public vrp::models::Depot,
                       public routing::attributes::Rendezvous {
            Depot(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::TW &timewindow) :
                    vrp::models::Depot(id, x, y),
                    routing::attributes::Rendezvous(timewindow) {
            }
        };
    }
}