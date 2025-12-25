// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include <memory>

namespace routing {
    namespace attributes {
        /**
         * @brief Service time required at a node
         *
         * ServiceQuery models the time duration required to perform
         * service at this node (loading, unloading, etc.).
         */
        struct ServiceQuery : public Attribute<ServiceQuery> {
            ServiceQuery(const Duration &p_service) : service(p_service) {}

            EntityData<Duration> service;

            Duration getService() const { return this->service.getValue(); }

            // IAttribute implementation
            std::unique_ptr<IAttribute> clone() const override {
                return std::make_unique<ServiceQuery>(getService());
            }

            std::string name() const override {
                return "ServiceQuery";
            }
        };
    }
}
