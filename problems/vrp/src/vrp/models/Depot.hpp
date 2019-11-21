//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

namespace vrp {
    namespace models {
        struct Depot
                : public routing::models::Depot, public routing::attributes::GeoNode {
            Depot(unsigned id, const routing::Duration &x, const routing::Duration &y) :
                    routing::models::Depot(id),
                    routing::attributes::GeoNode(x, y) {

            }
        };
    }
}