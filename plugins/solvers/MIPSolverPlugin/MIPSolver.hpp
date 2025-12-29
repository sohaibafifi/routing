// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ISolver.hpp"
#include "core/interfaces/IMIPBackend.hpp"
#include "core/interfaces/IMIPConstraintGenerator.hpp"
#include "core/PluginRegistry.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "plugins/attributes/RoutingPlugin/MIPRoutingGenerator.hpp"
#include "plugins/attributes/CapacityPlugin/MIPCapacityGenerator.hpp"
#include "plugins/attributes/TimeWindowPlugin/MIPTimeWindowGenerator.hpp"
#include "CPLEXMIPBackend.hpp"
#include "HiGHSMIPBackend.hpp"

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <typeindex>

namespace routing {

using namespace mip;

/**
 * @brief MIP-based solver using pluggable backends
 *
 * This solver uses the IMIPBackend interface to solve routing problems
 * using mixed integer programming. It supports different backends:
 * - CPLEX (commercial, high performance)
 * - HiGHS (open-source, good performance)
 * - Gurobi (future)
 */
class MIPSolver : public ISolver {
public:
    /**
     * @brief Construct a MIP solver for a problem
     * @param problem The problem to solve
     * @param backendType Backend to use ("cplex", "highs", "auto")
     *                    "auto" selects best available: CPLEX > HiGHS
     */
    explicit MIPSolver(Problem* problem, const std::string& backendType = "auto")
        : problem_(problem)
        , solution_(nullptr)
        , objectiveValue_(std::numeric_limits<double>::infinity())
        , timeout_(3600.0)
        , verbose_(false)
    {
        // Create the appropriate backend
        if (backendType == "cplex") {
            backend_ = std::make_unique<CPLEXMIPBackend>();
        } else if (backendType == "highs") {
            backend_ = std::make_unique<HiGHSMIPBackend>();
        } else if (backendType == "auto" || backendType == "mip") {
            // Auto-select best available backend
            backend_ = createBestBackend();
        } else {
            throw std::runtime_error("Unknown MIP backend: " + backendType +
                                   ". Available: cplex, highs, auto");
        }
    }

    /**
     * @brief Create the best available backend
     * Priority: CPLEX > HiGHS
     */
    static std::unique_ptr<IMIPBackend> createBestBackend() {
#ifdef CPLEX_FOUND
        try {
            return std::make_unique<CPLEXMIPBackend>();
        } catch (...) {}
#endif
#ifdef HIGHS_FOUND
        try {
            return std::make_unique<HiGHSMIPBackend>();
        } catch (...) {}
#endif
        throw std::runtime_error("No MIP backend available. Install CPLEX or HiGHS.");
    }

    /**
     * @brief Construct a MIP solver with a custom backend
     * @param problem The problem to solve
     * @param backend Custom backend instance
     */
    MIPSolver(Problem* problem, std::unique_ptr<IMIPBackend> backend)
        : problem_(problem)
        , backend_(std::move(backend))
        , solution_(nullptr)
        , objectiveValue_(std::numeric_limits<double>::infinity())
        , timeout_(3600.0)
        , verbose_(false)
    {}

    ~MIPSolver() override = default;

    std::string name() const override { return "mip"; }

    std::string description() const override {
        return "Mixed Integer Programming solver using " + backend_->name() + " backend";
    }

    void setProblem(Problem* problem) override {
        problem_ = problem;
        solution_ = nullptr;
    }

    Problem* getProblem() const override { return problem_; }

    void setConfiguration(Configuration* config) override {
        // TODO: Extract MIP-specific parameters from config
    }

    void setDefaultConfiguration() override {
        timeout_ = 3600.0;
        verbose_ = false;
    }

    bool solve(double timeout) override {
        if (!problem_) {
            if (verbose_) {
                std::cerr << "[MIPSolver] No problem set" << std::endl;
            }
            return false;
        }

        timeout_ = timeout;

        // Configure backend
        backend_->setTimeLimit(timeout);

        if (verbose_) {
            backend_->setVerbosity(4);
            std::cout << "[MIPSolver] Solving with " << backend_->name()
                      << " (timeout: " << timeout_ << "s)" << std::endl;
        } else {
            backend_->setVerbosity(0);
        }

        // Build the MIP model
        buildModel();

        // Solve
        MIPStatus status = backend_->solve(timeout_);

        if (verbose_) {
            std::cout << "[MIPSolver] Status: " << toString(status)
                      << ", Time: " << backend_->getSolveTime() << "s" << std::endl;
        }

        if (status == MIPStatus::Optimal || status == MIPStatus::Feasible ||
            status == MIPStatus::TimeLimit || status == MIPStatus::NodeLimit ||
            status == MIPStatus::SolutionLimit) {
            // Check if we actually have a solution
            try {
                objectiveValue_ = backend_->getObjectiveValue();
                extractSolution();
                return true;
            } catch (...) {
                // No solution available despite status
                return false;
            }
        }

        return false;
    }

    Solution* getSolution() const override { return solution_; }

    double getObjectiveValue() const override { return objectiveValue_; }

    bool isOptimal() const override {
        return backend_->getGap() < 1e-6;
    }

    std::string getStats() const override {
        std::ostringstream ss;
        ss << "Backend: " << backend_->name() << "\n"
           << "Time: " << backend_->getSolveTime() << "s\n"
           << "Nodes: " << backend_->getNumNodes() << "\n"
           << "Iterations: " << backend_->getNumIterations() << "\n"
           << "Objective: " << backend_->getObjectiveValue() << "\n"
           << "Bound: " << backend_->getObjectiveBound() << "\n"
           << "Gap: " << (backend_->getGap() * 100) << "%\n"
           << "Variables: " << backend_->getNumVars() << "\n"
           << "Constraints: " << backend_->getNumConstraints();
        return ss.str();
    }

    // ========== MIP-specific configuration ==========

    void setVerbose(bool verbose) { verbose_ = verbose; }
    void setGapTolerance(double gap) { backend_->setGapTolerance(gap); }
    void setNumThreads(int threads) { backend_->setNumThreads(threads); }
    void setNodeLimit(long long limit) { backend_->setNodeLimit(limit); }
    void setSolutionLimit(int limit) { backend_->setSolutionLimit(limit); }
    void setRandomSeed(int seed) { backend_->setRandomSeed(seed); }
    void setPresolve(bool enable) { backend_->setPresolve(enable); }

    /**
     * @brief Get the backend for advanced usage
     */
    IMIPBackend& getBackend() { return *backend_; }
    const IMIPBackend& getBackend() const { return *backend_; }

    /**
     * @brief Export the model to a file
     */
    bool exportModel(const std::string& filename) const {
        return backend_->exportModel(filename);
    }

protected:
    /**
     * @brief Build the MIP model using dynamically discovered constraint generators
     */
    virtual void buildModel() {
        backend_->clear();
        generators_.clear();
        autoGenerators_.clear();

        auto clients = problem_->getComposableClients();
        auto vehicles = problem_->getComposableVehicles();
        size_t n = clients.size();
        size_t m = vehicles.size();

        if (n == 0 || m == 0) return;

        // Get enabled attributes from the problem
        const auto& enabled = problem_->getEnabledAttributes();

        // Dynamically create generators from registry based on enabled attributes
        autoGenerators_ = PluginRegistry::instance().createMIPGenerators(enabled);

        if (verbose_) {
            std::cout << "[MIPSolver] Created " << autoGenerators_.size()
                      << " generators from registry" << std::endl;
        }

        // Find the routing generator and wire up dependencies
        mip::generators::MIPRoutingGenerator* routing = nullptr;
        for (auto& gen : autoGenerators_) {
            if (auto* routingPtr = dynamic_cast<mip::generators::MIPRoutingGenerator*>(gen.get())) {
                routing = routingPtr;
                break;
            }
        }

        // Wire up generators that depend on routing
        if (routing) {
            for (auto& gen : autoGenerators_) {
                if (auto* capacityGen = dynamic_cast<mip::generators::MIPCapacityGenerator*>(gen.get())) {
                    capacityGen->setRoutingGenerator(routing);
                } else if (auto* twGen = dynamic_cast<mip::generators::MIPTimeWindowGenerator*>(gen.get())) {
                    twGen->setRoutingGenerator(routing);
                }
            }
        }

        // Collect all generators
        for (const auto& gen : autoGenerators_) {
            generators_.push_back(gen.get());
        }

        if (generators_.empty()) {
            throw std::runtime_error("No MIP generators available - check that required attribute plugins are loaded");
        }

        // Sort generators by priority (already sorted by registry, but re-sort for consistency)
        std::sort(generators_.begin(), generators_.end(),
            [](const mip::IMIPConstraintGenerator* a, const mip::IMIPConstraintGenerator* b) {
                return a->priority() < b->priority();
            });

        if (verbose_) {
            std::cout << "[MIPSolver] Building model with " << generators_.size()
                      << " generators" << std::endl;
        }

        // Phase 1: Add variables
        for (auto* gen : generators_) {
            if (verbose_) {
                std::cout << "[MIPSolver] Adding variables for " << gen->name() << std::endl;
            }
            gen->addVariables(*backend_, *problem_);
        }

        // Phase 2: Add constraints
        for (auto* gen : generators_) {
            if (verbose_) {
                std::cout << "[MIPSolver] Adding constraints for " << gen->name() << std::endl;
            }
            gen->addConstraints(*backend_, *problem_);
        }

        // Phase 3: Build objective
        LinearExpr objective;
        for (auto* gen : generators_) {
            gen->addObjectiveTerms(*backend_, *problem_, objective);
        }

        backend_->minimize(objective);

        if (verbose_) {
            std::cout << "[MIPSolver] Model built: " << backend_->getNumVars() << " vars, "
                      << backend_->getNumConstraints() << " constraints" << std::endl;
        }
    }

    /**
     * @brief Extract solution from the MIP model using generators
     */
    virtual void extractSolution() {
        if (solution_) {
            delete solution_;
        }
        solution_ = new Solution(problem_);

        // Let generators extract their solution data
        for (auto* gen : generators_) {
            gen->extractSolution(*backend_, *problem_, *solution_);
        }

        if (solution_) {
            solution_->update();
        }
    }

    Problem* problem_;
    std::unique_ptr<IMIPBackend> backend_;
    Solution* solution_;
    double objectiveValue_;
    double timeout_;
    bool verbose_;

    // Constraint generators
    std::vector<std::unique_ptr<mip::IMIPConstraintGenerator>> autoGenerators_;
    std::vector<mip::IMIPConstraintGenerator*> generators_;
};

} // namespace routing
