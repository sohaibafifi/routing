// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"
#include "GeoNode.hpp"

namespace routing {
    namespace attributes {
        /**
         * @brief a node requiring a service
         */
        struct ServiceQuery {
            ServiceQuery(const Duration &p_service) : service(p_service) {}

            EntityData <Duration> service;

            Duration getService() const { return this->service.getValue(); }
        };
    }
}
