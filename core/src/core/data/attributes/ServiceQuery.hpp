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

            EntityData<Duration> service;

            Duration getService() const { return this->service.getValue(); }
        };
    }
}