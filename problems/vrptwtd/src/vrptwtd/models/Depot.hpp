// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrptw/models/Depot.hpp>
#include <core/data/attributes/Consumer.hpp>
#include <core/data/attributes/Rendezvous.hpp>
#include <core/data/attributes.hpp>

namespace vrptwtd::models {
        struct Depot : public cvrptw::models::Depot {
            Depot(unsigned id, const routing::Duration &x, const routing::Duration &y, const routing::TW &timewindow) :
                    cvrptw::models::Depot(id, x, y, timewindow){
            }
        };
    }