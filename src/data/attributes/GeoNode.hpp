#pragma once
#include "GeoNode.hpp"
#include "../attributes.hpp"
namespace routing {
    typedef double Duration;

    namespace attributes {

        /**
 * @brief a node with x y coordinates
 *
 */
        struct GeoNode
        {
                GeoNode(const Duration & p_x, const Duration & p_y):x(p_x), y(p_y){}
                EntityData<Duration> x;
                Duration getX()  const { return this->x.getValue();}
                EntityData<Duration> y;
                Duration getY()  const { return this->y.getValue();}
        };

    }
}
