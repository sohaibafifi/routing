//
// Created by OpenAI Codex CLI on 24/12/2025.
//

#include <gtest/gtest.h>

#include <core/data/AttributeRegistry.hpp>
#include <core/data/ComposableEntity.hpp>
#include <core/data/ComposableProblem.hpp>

#include <core/data/attributes/Consumer.hpp>
#include <core/data/attributes/GeoNode.hpp>
#include <core/data/attributes/Rendezvous.hpp>
#include <core/data/attributes/ServiceQuery.hpp>
#include <core/data/attributes/Stock.hpp>

#include <core/constraints/CapacityConstraintGenerator.hpp>
#include <core/constraints/RoutingConstraintGenerator.hpp>
#include <core/constraints/TimeWindowConstraintGenerator.hpp>

#include <memory>
#include <stdexcept>

namespace {
    using namespace routing;

    class DummyGenerator : public IConstraintGenerator {
    public:
        DummyGenerator(std::string name,
                       std::vector<AttributeTypeId> requiredAttrs,
                       int priority)
            : name_(std::move(name)),
              requiredAttrs_(std::move(requiredAttrs)),
              priority_(priority) {}

        std::string name() const override { return name_; }
        std::vector<AttributeTypeId> requiredAttributes() const override { return requiredAttrs_; }
        int priority() const override { return priority_; }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem&) override {}
        void addConstraints(ComposableProblem&) override {}
#endif

    private:
        std::string name_;
        std::vector<AttributeTypeId> requiredAttrs_;
        int priority_;
    };
} // namespace

TEST(ComposableEntityTest, attributeLifecycle) {
    routing::ComposableEntity entity(1);

    EXPECT_FALSE(entity.hasAttribute<routing::attributes::GeoNode>());
    EXPECT_EQ(entity.tryGetAttribute<routing::attributes::GeoNode>(), nullptr);
    EXPECT_THROW((void)entity.getAttribute<routing::attributes::GeoNode>(), std::runtime_error);

    entity.addAttribute<routing::attributes::GeoNode>(1.0, 2.0);
    EXPECT_TRUE(entity.hasAttribute<routing::attributes::GeoNode>());
    EXPECT_NE(entity.tryGetAttribute<routing::attributes::GeoNode>(), nullptr);
    EXPECT_DOUBLE_EQ(entity.getAttribute<routing::attributes::GeoNode>().getX(), 1.0);
    EXPECT_DOUBLE_EQ(entity.getAttribute<routing::attributes::GeoNode>().getY(), 2.0);

    entity.addAttribute<routing::attributes::Consumer>(5);
    EXPECT_TRUE(entity.hasAttribute<routing::attributes::Consumer>());
    EXPECT_EQ(entity.getAttribute<routing::attributes::Consumer>().getDemand(), 5);

    entity.addAttribute<routing::attributes::Consumer>(10);
    EXPECT_EQ(entity.getAttribute<routing::attributes::Consumer>().getDemand(), 10);

    EXPECT_TRUE(entity.removeAttribute<routing::attributes::Consumer>());
    EXPECT_FALSE(entity.hasAttribute<routing::attributes::Consumer>());
    EXPECT_EQ(entity.tryGetAttribute<routing::attributes::Consumer>(), nullptr);
    EXPECT_THROW((void)entity.getAttribute<routing::attributes::Consumer>(), std::runtime_error);
    EXPECT_FALSE(entity.removeAttribute<routing::attributes::Consumer>());
}

TEST(ComposableEntityTest, cloneDeepCopiesAttributes) {
    routing::ComposableEntity entity(7);
    entity.name = "entity-7";
    entity.addAttribute<routing::attributes::GeoNode>(3.0, 4.0);
    auto& consumer = entity.addAttribute<routing::attributes::Consumer>(12);
    consumer.setConsumption(9);

    auto copy = entity.clone();

    EXPECT_EQ(copy->getID(), 7u);
    EXPECT_EQ(copy->name, "entity-7");
    EXPECT_TRUE(copy->hasAttribute<routing::attributes::GeoNode>());
    EXPECT_TRUE(copy->hasAttribute<routing::attributes::Consumer>());
    EXPECT_NE(&copy->getAttribute<routing::attributes::Consumer>(), &entity.getAttribute<routing::attributes::Consumer>());
    EXPECT_EQ(copy->getAttribute<routing::attributes::Consumer>().getDemand(), 12);
    EXPECT_EQ(copy->getAttribute<routing::attributes::Consumer>().getConsumption(), 9);

    copy->getAttribute<routing::attributes::Consumer>().setConsumption(0);
    EXPECT_EQ(entity.getAttribute<routing::attributes::Consumer>().getConsumption(), 9);
}

TEST(ComposableProblemTest, usesGeoNodeForDistance) {
    routing::ComposableProblem problem;

    auto* depot = problem.addDepot(0);
    depot->addAttribute<routing::attributes::GeoNode>(0.0, 0.0);

    auto* c1 = problem.addClient(1);
    c1->addAttribute<routing::attributes::GeoNode>(3.0, 4.0);

    auto* c2 = problem.addClient(2);
    c2->addAttribute<routing::attributes::GeoNode>(6.0, 8.0);

    EXPECT_DOUBLE_EQ(problem.getDistance(*c1, *depot), 5.0);
    EXPECT_DOUBLE_EQ(problem.getDistance(*c1, *c2), 5.0);
    EXPECT_DOUBLE_EQ(problem.getDistanceComposable(*c1, *c2), 5.0);
}

TEST(ComposableProblemTest, activatesGeneratorsFromRegistry) {
    auto& registry = routing::AttributeRegistry::instance();
    registry.clear();

    registry.registerGenerator(std::make_unique<DummyGenerator>(
        "G_geo", std::vector<routing::AttributeTypeId>{std::type_index(typeid(routing::attributes::GeoNode))}, 20));
    registry.registerGenerator(std::make_unique<DummyGenerator>(
        "G_geo_cons",
        std::vector<routing::AttributeTypeId>{
            std::type_index(typeid(routing::attributes::GeoNode)),
            std::type_index(typeid(routing::attributes::Consumer)),
        },
        30));

    routing::ComposableProblem problem;
    EXPECT_TRUE(problem.getActiveGenerators().empty());

    problem.enableAttribute<routing::attributes::GeoNode>();
    ASSERT_EQ(problem.getActiveGenerators().size(), 1u);
    EXPECT_EQ(problem.getActiveGenerators()[0]->name(), "G_geo");

    problem.enableAttribute<routing::attributes::Consumer>();
    ASSERT_EQ(problem.getActiveGenerators().size(), 2u);
    EXPECT_EQ(problem.getActiveGenerators()[0]->name(), "G_geo");
    EXPECT_EQ(problem.getActiveGenerators()[1]->name(), "G_geo_cons");
}

TEST(ComposableProblemTest, realGeneratorsAreOrderedByPriority) {
    auto& registry = routing::AttributeRegistry::instance();
    registry.clear();

    registry.registerGenerator(std::make_unique<routing::constraints::TimeWindowConstraintGenerator>());
    registry.registerGenerator(std::make_unique<routing::constraints::CapacityConstraintGenerator>());
    registry.registerGenerator(std::make_unique<routing::constraints::RoutingConstraintGenerator>());

    routing::ComposableProblem problem;
    problem.enableAttributes<
        routing::attributes::GeoNode,
        routing::attributes::Consumer,
        routing::attributes::Stock,
        routing::attributes::Rendezvous,
        routing::attributes::ServiceQuery>();

    ASSERT_EQ(problem.getActiveGenerators().size(), 3u);
    EXPECT_EQ(problem.getActiveGenerators()[0]->name(), "RoutingConstraintGenerator");
    EXPECT_EQ(problem.getActiveGenerators()[1]->name(), "CapacityConstraintGenerator");
    EXPECT_EQ(problem.getActiveGenerators()[2]->name(), "TimeWindowConstraintGenerator");
}
