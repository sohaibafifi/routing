// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once
//#include "GeoNode.hpp"
#include "core/data/attributes.hpp"

namespace routing {
    typedef double Duration;

    namespace attributes {

        /**
 * @brief a node with x y coordinates
 *
 */
        struct GeoNode {
            GeoNode(const Duration &p_x, const Duration &p_y) : x(p_x), y(p_y) {}

            EntityData <Duration> x;

            Duration getX() const { return this->x.getValue(); }

            EntityData <Duration> y;

            Duration getY() const { return this->y.getValue(); }
        };

    }
}
