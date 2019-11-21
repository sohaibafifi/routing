//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <core/data/models/Tour.hpp>

namespace vrp {
    namespace models {

        class Tour : public routing::models::Tour {

            routing::Duration traveltime;
        public:
            routing::Duration getTraveltime() const {
                return traveltime;
            }
        };
    }
}

