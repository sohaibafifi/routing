// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IEvaluator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"

#include <typeindex>
#include <vector>

namespace routing {
namespace evaluators {

class RoutingEvaluator : public IEvaluator {
public:
    std::string name() const override { return "RoutingEvaluator"; }

    std::vector<AttributeTypeId> requiredAttributes() const override {
        return { std::type_index(typeid(attributes::GeoNode)) };
    }

    int priority() const override { return 10; }

    bool checkFeasibility(const Tour& /*tour*/,
                          const InsertionContext& /*ctx*/) const override {
        return true;
    }

    double evaluateInsertionDelta(const Tour& tour,
                                  const InsertionContext& ctx) const override {
        auto* problem = tour.getProblem();
        if (!problem || !ctx.client || !ctx.predecessor || !ctx.successor) {
            return 0.0;
        }

        const double direct = problem->getDistanceEntity(*ctx.predecessor, *ctx.successor);
        const double via = problem->getDistanceEntity(*ctx.predecessor, *ctx.client)
                         + problem->getDistanceEntity(*ctx.client, *ctx.successor);
        return via - direct;
    }

    void applyInsertion(Tour& /*tour*/,
                        const InsertionContext& /*ctx*/) override {}
};

} // namespace evaluators
} // namespace routing
