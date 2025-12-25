// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <vrp/models/Depot.hpp>
#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/TimeWindowPlugin/Rendezvous.hpp>
#include <core/interfaces/IAttribute.hpp>

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