//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

namespace vrp { namespace models {
    struct Client
            : public routing::models::Client, public routing::attributes::GeoNode {
        Client(unsigned id, const routing::Duration &x, const routing::Duration &y) :
                routing::models::Client(id),
                routing::attributes::GeoNode(x, y) {
        }
    };
} }