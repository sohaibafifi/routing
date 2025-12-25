#include <gtest/gtest.h>

#include <core/PluginRegistry.hpp>
#include <core/IPlugin.hpp>
#include <core/interfaces/IConstraintGenerator.hpp>
#include <core/interfaces/IEvaluator.hpp>
#include <core/interfaces/ISolver.hpp>
#include <core/interfaces/INeighborhood.hpp>
#include <core/interfaces/IReader.hpp>
#include <plugins/attributes/ComposableCorePlugin/Problem.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace {
    struct DummyAttribute : public routing::Attribute<DummyAttribute> {
        std::unique_ptr<routing::IAttribute> clone() const override {
            return std::make_unique<DummyAttribute>();
        }
        std::string name() const override { return "DummyAttribute"; }
    };

    class DummyGenerator : public routing::IConstraintGenerator {
    public:
        std::string name() const override { return "DummyGenerator"; }
        std::vector<routing::AttributeTypeId> requiredAttributes() const override {
            return {std::type_index(typeid(DummyAttribute))};
        }
        int priority() const override { return 10; }
#ifdef CPLEX_FOUND
        void addVariables(routing::ComposableProblem&) override {}
        void addConstraints(routing::ComposableProblem&) override {}
#endif
    };

    class DummyEvaluator : public routing::IEvaluator {
    public:
        std::string name() const override { return "DummyEvaluator"; }
        std::vector<routing::AttributeTypeId> requiredAttributes() const override {
            return {std::type_index(typeid(DummyAttribute))};
        }
        bool checkFeasibility(const routing::Tour&, const routing::InsertionContext&) const override {
            return true;
        }
        double evaluateInsertionDelta(const routing::Tour&, const routing::InsertionContext&) const override {
            return 0.0;
        }
        void applyInsertion(routing::Tour&, const routing::InsertionContext&) override {}
    };

    class DummySolver : public routing::ISolver {
    public:
        std::string name() const override { return "dummy_solver"; }
        void setProblem(routing::Problem* problem) override { problem_ = problem; }
        routing::Problem* getProblem() const override { return problem_; }
        void setConfiguration(routing::Configuration* config) override { config_ = config; }
        void setDefaultConfiguration() override {}
        bool solve(double) override { return true; }
        routing::Solution* getSolution() const override { return nullptr; }
        double getObjectiveValue() const override { return 0.0; }

    private:
        routing::Problem* problem_ = nullptr;
        routing::Configuration* config_ = nullptr;
    };

    class DummyNeighborhood : public routing::INeighborhood {
    public:
        std::string name() const override { return "dummy_neighborhood"; }
        std::optional<routing::NeighborhoodMove> explore(routing::Solution&) override {
            return std::nullopt;
        }
        void apply(routing::Solution&, const routing::NeighborhoodMove&) override {}
    };

    class DummyReader : public routing::IReader {
    public:
        std::string formatName() const override { return "dummy"; }
        std::vector<std::string> supportedExtensions() const override { return {".dummy"}; }
        bool canRead(const std::string&) const override { return true; }
        routing::Problem* readFile(const std::string&) override { return nullptr; }
    };

    class TestPlugin : public routing::IPlugin {
    public:
        TestPlugin(std::string name, std::vector<std::string> deps, std::vector<std::string>* order)
            : name_(std::move(name)), deps_(std::move(deps)), order_(order) {}

        std::string name() const override { return name_; }
        routing::PluginType type() const override { return routing::PluginType::Attribute; }
        std::vector<std::string> dependencies() const override { return deps_; }
        void initialize(routing::PluginRegistry&) override {
            if (order_) {
                order_->push_back(name_);
            }
        }

    private:
        std::string name_;
        std::vector<std::string> deps_;
        std::vector<std::string>* order_ = nullptr;
    };
}

TEST(PluginRegistryTest, resolvesDependenciesInOrder) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();

    std::vector<std::string> order;

    registry.registerPlugin(std::make_unique<TestPlugin>("PluginB", std::vector<std::string>{"PluginA"}, &order));
    registry.registerPlugin(std::make_unique<TestPlugin>("PluginA", std::vector<std::string>{}, &order));

    registry.initializeAll();

    auto itA = std::find(order.begin(), order.end(), "PluginA");
    auto itB = std::find(order.begin(), order.end(), "PluginB");

    ASSERT_NE(itA, order.end());
    ASSERT_NE(itB, order.end());
    EXPECT_LT(itA, itB);
}

TEST(PluginRegistryTest, detectsCircularDependencies) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();

    registry.registerPlugin(std::make_unique<TestPlugin>("PluginX", std::vector<std::string>{"PluginY"}, nullptr));
    registry.registerPlugin(std::make_unique<TestPlugin>("PluginY", std::vector<std::string>{"PluginX"}, nullptr));

    EXPECT_THROW(registry.initializeAll(), std::runtime_error);
}

TEST(PluginRegistryTest, createsSolverByName) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();

    registry.registerSolver("dummy",
        [](routing::Problem*) -> std::unique_ptr<routing::ISolver> {
            return std::make_unique<DummySolver>();
        });

    auto solver = registry.createSolver("dummy", nullptr);
    EXPECT_EQ(solver->name(), "dummy_solver");
}

TEST(PluginRegistryTest, createsNeighborhoodByName) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();

    registry.registerNeighborhood("dummy",
        []() -> std::unique_ptr<routing::INeighborhood> {
            return std::make_unique<DummyNeighborhood>();
        });

    auto neighborhood = registry.createNeighborhood("dummy");
    EXPECT_EQ(neighborhood->name(), "dummy_neighborhood");
}

TEST(PluginRegistryTest, createsReaderByExtension) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();

    registry.registerReader("dummy", {".dummy"}, []() -> std::unique_ptr<routing::IReader> {
        return std::make_unique<DummyReader>();
    });

    auto reader = registry.createReaderForExtension(".dummy");
    EXPECT_EQ(reader->formatName(), "dummy");
}

TEST(PluginRegistryTest, listsGeneratorsAndEvaluators) {
    auto& registry = routing::PluginRegistry::instance();
    registry.clear();

    registry.registerGenerator(std::make_unique<DummyGenerator>());
    registry.registerEvaluator(std::make_unique<DummyEvaluator>());

    auto generators = registry.availableGenerators();
    auto evaluators = registry.availableEvaluators();

    EXPECT_NE(std::find(generators.begin(), generators.end(), "DummyGenerator"), generators.end());
    EXPECT_NE(std::find(evaluators.begin(), evaluators.end(), "DummyEvaluator"), evaluators.end());
}
