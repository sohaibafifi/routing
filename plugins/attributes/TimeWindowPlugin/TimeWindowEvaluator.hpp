// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IEvaluator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"

#include <algorithm>
#include <typeindex>
#include <vector>

namespace routing {
namespace evaluators {

class TimeWindowEvaluator : public IEvaluator {
public:
    std::string name() const override { return "TimeWindowEvaluator"; }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return {
            std::type_index(typeid(attributes::Rendezvous)),
            std::type_index(typeid(attributes::ServiceQuery))
        };
    }

    int priority() const override { return 60; }

    bool checkFeasibility(const Tour& tour,
                          const InsertionContext& ctx) const override {
        auto* problem = tour.getProblem();
        if (!problem || !ctx.client) {
            return true;
        }

        auto* depot = problem->getDepot();
        auto* depotEntity = dynamic_cast<Entity*>(depot);
        if (!depotEntity) {
            return true;
        }

        double depotOpen = 0.0;
        double depotClose = 1e9;
        if (auto* depotTw = depotEntity->tryGetAttribute<attributes::Rendezvous>()) {
            depotOpen = depotTw->getTwOpen();
            depotClose = depotTw->getTwClose();
        }

        auto getWindow = [](Entity* entity) {
            double open = 0.0;
            double close = 1e9;
            if (entity) {
                if (auto* tw = entity->tryGetAttribute<attributes::Rendezvous>()) {
                    open = tw->getTwOpen();
                    close = tw->getTwClose();
                }
            }
            return std::pair<double, double>(open, close);
        };

        auto getService = [](Entity* entity) -> double {
            if (entity) {
                if (auto* service = entity->tryGetAttribute<attributes::ServiceQuery>()) {
                    return service->getService();
                }
            }
            return 0.0;
        };

        std::vector<Entity*> route;
        route.reserve(tour.getNbClient() + 1);
        for (unsigned long i = 0; i < tour.getNbClient(); ++i) {
            route.push_back(dynamic_cast<Entity*>(tour.getClient(i)));
        }

        size_t insertPos = 0;
        if (ctx.position > 0) {
            insertPos = static_cast<size_t>(ctx.position);
        }
        if (insertPos > route.size()) {
            insertPos = route.size();
        }
        route.insert(route.begin() + insertPos, ctx.client);

        double time = depotOpen;
        Entity* prev = depotEntity;
        for (auto* node : route) {
            if (!node) {
                continue;
            }
            const double travel = problem->getDistanceEntity(*prev, *node);
            const double arrival = time + travel;
            const auto [open, close] = getWindow(node);
            const double start = std::max(arrival, open);
            if (start > close + 1e-9) {
                return false;
            }
            time = start + getService(node);
            prev = node;
        }

        const double returnTime = time + problem->getDistanceEntity(*prev, *depotEntity);
        return returnTime <= depotClose + 1e-9;
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
