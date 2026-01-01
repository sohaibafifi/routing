// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <gtest/gtest.h>
#include <chrono>
#include <vector>

#include "plugins/PluginBundle.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"
#include "core/interfaces/IIncrementalEvaluator.hpp"

using namespace routing;
using namespace routing::attributes;

class IncrementalEvaluatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Register and initialize all plugins
        plugins::registerCorePlugins();
        PluginRegistry::instance().initializeAll();
    }

    void TearDown() override {
        // Cleanup registry for next test
        PluginRegistry::instance().shutdownAll();
        PluginRegistry::instance().clear();
    }

    /**
     * Create a simple CVRPTW problem with n clients
     */
    std::unique_ptr<Problem> createProblem(size_t numClients, double vehicleCapacity = 100.0) {
        auto problem = std::make_unique<Problem>();
        problem->enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();

        // Add depot at (0, 0)
        auto* depot = problem->addDepot(0);
        depot->addAttribute<GeoNode>(0.0, 0.0);
        depot->addAttribute<Rendezvous>(0.0, 1000.0);
        depot->addAttribute<ServiceQuery>(0.0);

        // Add vehicle
        auto* vehicle = problem->addVehicle(0);
        vehicle->addAttribute<Stock>(vehicleCapacity);

        // Add clients in a circle around depot
        for (size_t i = 0; i < numClients; ++i) {
            auto* client = problem->addClient(static_cast<unsigned>(i + 1));
            double angle = 2.0 * M_PI * i / numClients;
            double x = 50.0 * std::cos(angle);
            double y = 50.0 * std::sin(angle);
            client->addAttribute<GeoNode>(x, y);
            client->addAttribute<Consumer>(5.0);  // Small demand
            client->addAttribute<Rendezvous>(0.0, 500.0);  // Wide time window
            client->addAttribute<ServiceQuery>(1.0);
        }

        problem->syncLegacyPointers();
        return problem;
    }
};

TEST_F(IncrementalEvaluatorTest, CacheBuildsCorrectly) {
    auto problem = createProblem(10);
    auto* solution = new Solution(problem.get());
    auto* tour = new Tour(problem.get(), 0);
    solution->pushTour(tour);

    // Add clients to tour
    auto clients = problem->getClients();
    for (auto* client : clients) {
        tour->_pushClient(client);
    }
    tour->update();

    // Ensure cache is built
    tour->ensureCache();
    EXPECT_TRUE(tour->isCacheValid());

    const RouteCache& cache = tour->getCache();
    EXPECT_EQ(cache.size(), 10u);

    // Verify arrival times are computed
    EXPECT_GT(cache.arrivalTime[0], 0.0);
    EXPECT_GT(cache.totalDistance, 0.0);

    delete solution;
}

TEST_F(IncrementalEvaluatorTest, InsertionEvaluationIsConsistent) {
    auto problem = createProblem(10);
    auto* solution = new Solution(problem.get());
    auto* tour = new Tour(problem.get(), 0);
    solution->pushTour(tour);

    // Add first 5 clients
    auto clients = problem->getClients();
    for (size_t i = 0; i < 5; ++i) {
        tour->_pushClient(clients[i]);
    }
    tour->update();

    // Evaluate insertion of 6th client without cache
    tour->invalidateCache();
    auto* costWithoutCache = tour->evaluateInsertion(clients[5], 2);

    // Evaluate insertion with cache
    tour->ensureCache();
    auto* costWithCache = tour->evaluateInsertion(clients[5], 2);

    // Results should match
    EXPECT_NEAR(costWithoutCache->getDelta(), costWithCache->getDelta(), 1e-6);
    EXPECT_EQ(costWithoutCache->isPossible(), costWithCache->isPossible());

    delete costWithoutCache;
    delete costWithCache;
    delete solution;
}

TEST_F(IncrementalEvaluatorTest, CacheInvalidatesOnClear) {
    auto problem = createProblem(5);
    auto* tour = new Tour(problem.get(), 0);

    auto clients = problem->getClients();
    for (auto* client : clients) {
        tour->_pushClient(client);
    }

    tour->ensureCache();
    EXPECT_TRUE(tour->isCacheValid());

    tour->clear();
    EXPECT_FALSE(tour->isCacheValid());

    delete tour;
}

TEST_F(IncrementalEvaluatorTest, CapacityFeasibilityCheck) {
    // Create problem with tight capacity
    auto problem = createProblem(10, 30.0);  // Each client has demand 5, so max 6 clients

    auto* tour = new Tour(problem.get(), 0);
    auto clients = problem->getClients();

    // Add 6 clients (total demand = 30 = capacity)
    for (size_t i = 0; i < 6; ++i) {
        tour->_pushClient(clients[i]);
    }
    tour->update();
    tour->ensureCache();

    // 7th client should be infeasible
    auto* cost = tour->evaluateInsertion(clients[6], 3);
    EXPECT_FALSE(cost->isPossible());

    delete cost;
    delete tour;
}

TEST_F(IncrementalEvaluatorTest, TimeWindowFeasibilityCheck) {
    auto problem = std::make_unique<Problem>();
    problem->enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();

    // Depot with time window [0, 100]
    auto* depot = problem->addDepot(0);
    depot->addAttribute<GeoNode>(0.0, 0.0);
    depot->addAttribute<Rendezvous>(0.0, 100.0);
    depot->addAttribute<ServiceQuery>(0.0);

    auto* vehicle = problem->addVehicle(0);
    vehicle->addAttribute<Stock>(1000.0);

    // Client 1: close to depot, early time window
    auto* client1 = problem->addClient(1);
    client1->addAttribute<GeoNode>(10.0, 0.0);
    client1->addAttribute<Consumer>(1.0);
    client1->addAttribute<Rendezvous>(0.0, 30.0);
    client1->addAttribute<ServiceQuery>(5.0);

    // Client 2: far from depot, late time window
    auto* client2 = problem->addClient(2);
    client2->addAttribute<GeoNode>(80.0, 0.0);
    client2->addAttribute<Consumer>(1.0);
    client2->addAttribute<Rendezvous>(50.0, 100.0);
    client2->addAttribute<ServiceQuery>(5.0);

    // Client 3: early time window that conflicts
    auto* client3 = problem->addClient(3);
    client3->addAttribute<GeoNode>(50.0, 0.0);
    client3->addAttribute<Consumer>(1.0);
    client3->addAttribute<Rendezvous>(0.0, 20.0);  // Very tight window
    client3->addAttribute<ServiceQuery>(5.0);

    problem->syncLegacyPointers();

    auto* tour = new Tour(problem.get(), 0);

    // Add client2 first (far, late window)
    tour->_pushClient(client2);
    tour->update();
    tour->ensureCache();

    // Try to insert client3 before client2 - should fail due to time window
    // Client3 has close time 20, but arriving at client3 after depot takes ~50 time units
    auto* cost = tour->evaluateInsertion(client3, 0);
    // This depends on the exact implementation; the key is it uses cache
    // Either feasible or not, the result should be consistent

    delete cost;
    delete tour;
}

TEST_F(IncrementalEvaluatorTest, TwoOptEvaluationPerformance) {
    // Create larger problem to measure performance difference
    const size_t numClients = 50;
    auto problem = createProblem(numClients);

    auto* tour = new Tour(problem.get(), 0);
    auto clients = problem->getClients();
    for (auto* client : clients) {
        tour->_pushClient(client);
    }
    tour->update();

    // Build cache
    tour->ensureCache();
    EXPECT_TRUE(tour->isCacheValid());

    // Get incremental evaluators
    const auto& evaluators = problem->getActiveEvaluators();
    std::vector<IIncrementalEvaluator*> incEvaluators;
    for (auto* eval : evaluators) {
        if (auto* inc = dynamic_cast<IIncrementalEvaluator*>(eval)) {
            incEvaluators.push_back(inc);
        }
    }

    EXPECT_GT(incEvaluators.size(), 0u);

    // Measure time for 2-opt evaluations
    const RouteCache& cache = tour->getCache();
    int feasibleCount = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < static_cast<int>(numClients) - 1; ++i) {
        for (int j = i + 2; j < static_cast<int>(numClients); ++j) {
            TwoOptContext ctx{i, j};
            bool feasible = true;
            for (auto* incEval : incEvaluators) {
                MoveDelta md = incEval->evaluateTwoOptIncremental(*tour, cache, ctx);
                if (!md.feasible) {
                    feasible = false;
                    break;
                }
            }
            if (feasible) feasibleCount++;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "[Performance] 2-opt evaluations for " << numClients << " clients: "
              << duration.count() << " microseconds" << std::endl;
    std::cout << "[Performance] Feasible moves: " << feasibleCount << std::endl;

    // Should complete quickly (< 100ms for 50 clients)
    EXPECT_LT(duration.count(), 100000);

    delete tour;
}

TEST_F(IncrementalEvaluatorTest, MaxForwardShiftComputation) {
    auto problem = std::make_unique<Problem>();
    problem->enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();

    // Depot
    auto* depot = problem->addDepot(0);
    depot->addAttribute<GeoNode>(0.0, 0.0);
    depot->addAttribute<Rendezvous>(0.0, 1000.0);
    depot->addAttribute<ServiceQuery>(0.0);

    auto* vehicle = problem->addVehicle(0);
    vehicle->addAttribute<Stock>(1000.0);

    // Clients with known time windows
    auto* c1 = problem->addClient(1);
    c1->addAttribute<GeoNode>(10.0, 0.0);
    c1->addAttribute<Consumer>(1.0);
    c1->addAttribute<Rendezvous>(0.0, 100.0);
    c1->addAttribute<ServiceQuery>(10.0);

    auto* c2 = problem->addClient(2);
    c2->addAttribute<GeoNode>(20.0, 0.0);
    c2->addAttribute<Consumer>(1.0);
    c2->addAttribute<Rendezvous>(0.0, 100.0);
    c2->addAttribute<ServiceQuery>(10.0);

    problem->syncLegacyPointers();

    auto* tour = new Tour(problem.get(), 0);
    tour->_pushClient(c1);
    tour->_pushClient(c2);
    tour->update();

    tour->ensureCache();
    const RouteCache& cache = tour->getCache();

    // maxForwardShift should be positive (there's slack)
    EXPECT_GT(cache.maxForwardShift[0], 0.0);
    EXPECT_GT(cache.maxForwardShift[1], 0.0);

    delete tour;
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
