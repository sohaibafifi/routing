//
// Created by OpenAI Codex CLI on 24/12/2025.
//

#include <gtest/gtest.h>

#include <core/PluginRegistry.hpp>
#include <plugins/attributes/ComposableCorePlugin/Entity.hpp>
#include <plugins/attributes/ComposableCorePlugin/Problem.hpp>

#include <plugins/attributes/CapacityPlugin/Consumer.hpp>
#include <plugins/attributes/RoutingPlugin/GeoNode.hpp>
#include <plugins/attributes/TimeWindowPlugin/Rendezvous.hpp>
#include <plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp>
#include <plugins/attributes/CapacityPlugin/Stock.hpp>

#include <examples/problems/cvrp/Reader.hpp>
#include <examples/problems/cvrptw/Reader.hpp>

#include <plugins/attributes/CapacityPlugin/CapacityConstraintGenerator.hpp>
#include <plugins/attributes/RoutingPlugin/RoutingConstraintGenerator.hpp>
#include <plugins/attributes/TimeWindowPlugin/TimeWindowConstraintGenerator.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
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

    class TempFile {
    public:
        TempFile(const std::string& contents, const std::string& suffix) {
            static std::atomic<unsigned long long> counter{0};
            auto id = counter.fetch_add(1);
            auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            path_ = std::filesystem::temp_directory_path() /
                    ("routing_test_" + std::to_string(stamp) + "_" + std::to_string(id) + suffix);
            std::ofstream out(path_);
            out << contents;
        }

        ~TempFile() {
            std::error_code ec;
            std::filesystem::remove(path_, ec);
        }

        const std::filesystem::path& path() const { return path_; }

    private:
        std::filesystem::path path_;
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
    auto& registry = routing::PluginRegistry::instance();
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
    auto& registry = routing::PluginRegistry::instance();
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

TEST(ComposableCVRPReaderTest, parsesTsplibIntoComposableProblem) {
    const std::string content =
        "NAME : A-n3-k2\n"
        "TYPE : CVRP\n"
        "DIMENSION : 3\n"
        "CAPACITY : 50\n"
        "VEHICLES : 2\n"
        "NODE_COORD_SECTION\n"
        "1 0 0\n"
        "2 3 4\n"
        "3 6 8\n"
        "DEMAND_SECTION\n"
        "1 0\n"
        "2 10\n"
        "3 20\n"
        "DEPOT_SECTION\n"
        "1\n"
        "-1\n"
        "EOF\n";

    TempFile file(content, ".vrp");
    composable::cvrp::Reader reader;
    std::unique_ptr<routing::Problem> problem(reader.readFile(file.path().string()));

    ASSERT_NE(problem, nullptr);
    EXPECT_EQ(problem->getName(), "A-n3-k2");
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::GeoNode>());
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::Consumer>());
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::Stock>());
    EXPECT_EQ(problem->numVehicles(), 2u);
    EXPECT_EQ(problem->numDepots(), 1u);
    EXPECT_EQ(problem->numClients(), 2u);

    auto* typed = dynamic_cast<composable::cvrp::Problem*>(problem.get());
    ASSERT_NE(typed, nullptr);

    auto* depot = problem->getDepot();
    ASSERT_NE(depot, nullptr);
    auto* depotGeo = depot->tryGetAttribute<routing::attributes::GeoNode>();
    ASSERT_NE(depotGeo, nullptr);
    EXPECT_DOUBLE_EQ(depotGeo->getX(), 0.0);
    EXPECT_DOUBLE_EQ(depotGeo->getY(), 0.0);

    routing::Client* client2 = nullptr;
    routing::Client* client3 = nullptr;
    for (auto* client : problem->getClients()) {
        if (client->getID() == 2) {
            client2 = client;
        } else if (client->getID() == 3) {
            client3 = client;
        }
    }
    ASSERT_NE(client2, nullptr);
    ASSERT_NE(client3, nullptr);
    auto* demand2 = client2->tryGetAttribute<routing::attributes::Consumer>();
    auto* demand3 = client3->tryGetAttribute<routing::attributes::Consumer>();
    ASSERT_NE(demand2, nullptr);
    ASSERT_NE(demand3, nullptr);
    EXPECT_EQ(demand2->getDemand(), 10);
    EXPECT_EQ(demand3->getDemand(), 20);

    auto vehicles = problem->getVehicles();
    ASSERT_EQ(vehicles.size(), 2u);
    for (auto* vehicle : vehicles) {
        auto* stock = vehicle->tryGetAttribute<routing::attributes::Stock>();
        ASSERT_NE(stock, nullptr);
        EXPECT_DOUBLE_EQ(stock->getCapacity(), 50.0);
    }
}

TEST(ComposableCVRPTWReaderTest, parsesSolomonIntoComposableProblem) {
    const std::string content =
        "C101\n"
        "COMMENT\n"
        "COMMENT\n"
        "COMMENT\n"
        "3 100\n"
        "HEADER\n"
        "HEADER\n"
        "HEADER\n"
        "HEADER\n"
        "0 0 0 0 0 1000 0\n"
        "1 10 10 10 0 100 5\n"
        "2 20 20 20 10 120 5\n";

    TempFile file(content, ".txt");
    composable::cvrptw::Reader reader;
    std::unique_ptr<routing::Problem> problem(reader.readFile(file.path().string()));

    ASSERT_NE(problem, nullptr);
    EXPECT_EQ(problem->getName(), "C101");
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::GeoNode>());
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::Consumer>());
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::Stock>());
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::Rendezvous>());
    EXPECT_TRUE(problem->hasAttribute<routing::attributes::ServiceQuery>());
    EXPECT_EQ(problem->numVehicles(), 3u);
    EXPECT_EQ(problem->numDepots(), 1u);
    EXPECT_EQ(problem->numClients(), 2u);

    auto* typed = dynamic_cast<composable::cvrptw::Problem*>(problem.get());
    ASSERT_NE(typed, nullptr);

    auto* depot = problem->getDepot();
    ASSERT_NE(depot, nullptr);
    auto* depotTw = depot->tryGetAttribute<routing::attributes::Rendezvous>();
    ASSERT_NE(depotTw, nullptr);
    EXPECT_DOUBLE_EQ(depotTw->getTwOpen(), 0.0);
    EXPECT_DOUBLE_EQ(depotTw->getTwClose(), 1000.0);

    routing::Client* client1 = nullptr;
    for (auto* client : problem->getClients()) {
        if (client->getID() == 1) {
            client1 = client;
        }
    }
    ASSERT_NE(client1, nullptr);
    auto* demand1 = client1->tryGetAttribute<routing::attributes::Consumer>();
    auto* tw1 = client1->tryGetAttribute<routing::attributes::Rendezvous>();
    auto* svc1 = client1->tryGetAttribute<routing::attributes::ServiceQuery>();
    ASSERT_NE(demand1, nullptr);
    ASSERT_NE(tw1, nullptr);
    ASSERT_NE(svc1, nullptr);
    EXPECT_EQ(demand1->getDemand(), 10);
    EXPECT_DOUBLE_EQ(tw1->getTwOpen(), 0.0);
    EXPECT_DOUBLE_EQ(tw1->getTwClose(), 100.0);
    EXPECT_DOUBLE_EQ(svc1->getService(), 5.0);
}
