// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IEvaluator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"

#include <cmath>
#include <limits>
#include <typeindex>
#include <vector>

namespace routing {
namespace evaluators {

class CapacityEvaluator : public IEvaluator {
public:
    std::string name() const override { return "CapacityEvaluator"; }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return {
            std::type_index(typeid(attributes::Consumer)),
            std::type_index(typeid(attributes::Stock))
        };
    }

    int priority() const override { return 50; }

    bool checkFeasibility(const Tour& tour,
                          const InsertionContext& ctx) const override {
        auto* problem = tour.getProblem();
        if (!problem || !ctx.client) {
            return true;
        }

        double capacity = std::numeric_limits<double>::infinity();
        auto vehicles = problem->getVehicles();
        Vehicle* vehicle = nullptr;
        for (auto* v : vehicles) {
            if (v && v->getID() == tour.getID()) {
                vehicle = v;
                break;
            }
        }
        if (!vehicle && !vehicles.empty()) {
            vehicle = vehicles.front();
        }
        if (vehicle) {
            if (auto* stock = vehicle->tryGetAttribute<attributes::Stock>()) {
                capacity = stock->getCapacity();
            }
        }

        if (!std::isfinite(capacity)) {
            return true;
        }

        double load = 0.0;
        for (unsigned long i = 0; i < tour.getNbClient(); ++i) {
            auto* client = tour.getClient(i);
            auto* entity = dynamic_cast<Entity*>(client);
            if (entity) {
                if (auto* consumer = entity->tryGetAttribute<attributes::Consumer>()) {
                    load += consumer->getDemand();
                }
            }
        }

        if (auto* consumer = ctx.client->tryGetAttribute<attributes::Consumer>()) {
            load += consumer->getDemand();
        }

        return load <= capacity + 1e-9;
    }

    double evaluateInsertionDelta(const Tour& /*tour*/,
                                  const InsertionContext& /*ctx*/) const override {
        return 0.0;
    }

    void applyInsertion(Tour& /*tour*/,
                        const InsertionContext& /*ctx*/) override {}
};

} // namespace evaluators
} // namespace routing
