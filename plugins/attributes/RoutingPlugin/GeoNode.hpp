// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include <memory>
#include <cmath>

namespace routing {
    typedef double Duration;

    namespace attributes {

        /**
         * @brief A node with x, y coordinates
         *
         * GeoNode provides geographic positioning for entities.
         * Used for distance calculations between nodes.
         */
        struct GeoNode : public Attribute<GeoNode> {
            GeoNode(const Duration &p_x, const Duration &p_y) : x(p_x), y(p_y) {}

            EntityData<Duration> x;
            EntityData<Duration> y;

            Duration getX() const { return this->x.getValue(); }
            Duration getY() const { return this->y.getValue(); }

            /// Compute Euclidean distance to another GeoNode
            Duration distanceTo(const GeoNode& other) const {
                Duration dx = getX() - other.getX();
                Duration dy = getY() - other.getY();
                return std::sqrt(dx * dx + dy * dy);
            }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<GeoNode>(getX(), getY());
            }

            std::string name() const override {
                return "GeoNode";
            }
        };

    }
}
