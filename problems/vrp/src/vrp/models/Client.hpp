// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

namespace vrp {
    namespace models {
        struct Client
                : public routing::models::Client, public routing::attributes::GeoNode {
            Client(unsigned id, const routing::Duration &x, const routing::Duration &y) :
                    routing::models::Client(id),
                    routing::attributes::GeoNode(x, y) {
            }
        };
    }
}