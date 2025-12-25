// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ISolver.hpp"
#include "core/interfaces/ICPBackend.hpp"
#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "CPOptimizerBackend.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

namespace routing {
namespace cp {

/**
 * @brief CP-based solver using pluggable backends
 *
 * This solver uses the ICPBackend interface to solve routing problems
 * using constraint programming. It automatically discovers and uses
 * ICPConstraintGenerator instances based on the problem's enabled attributes.
 */
class CPSolver : public ISolver {
public:
    /**
     * @brief Construct a CP solver for a problem
     * @param problem The problem to solve
     * @param backendType Backend to use ("cpoptimizer", "ortools", "gecode")
     */
    explicit CPSolver(Problem* problem, const std::string& backendType = "cpoptimizer")
        : problem_(problem)
        , solution_(nullptr)
        , objectiveValue_(std::numeric_limits<double>::infinity())
        , timeout_(3600.0)
        , numWorkers_(0)
        , verbose_(false)
    {
        // Create the appropriate backend
        if (backendType == "cpoptimizer" || backendType == "cp" || backendType == "cpo") {
            backend_ = std::make_unique<CPOptimizerBackend>();
        } else {
            throw std::runtime_error("Unknown CP backend: " + backendType +
                                   ". Available: cpoptimizer");
        }
    }

    ~CPSolver() override = default;

    std::string name() const override { return "cp"; }

    std::string description() const override {
        return "Constraint Programming solver using " + backend_->name() + " backend";
    }

    void setProblem(Problem* problem) override {
        problem_ = problem;
        solution_ = nullptr;
    }

    Problem* getProblem() const override { return problem_; }

    void setConfiguration(Configuration* config) override {
        // TODO: Extract CP-specific parameters from config
    }

    void setDefaultConfiguration() override {
        timeout_ = 3600.0;
        numWorkers_ = 0;  // Use backend default
        verbose_ = false;
    }

    bool solve(double timeout) override {
        if (!problem_) {
            std::cerr << "[CPSolver] No problem set" << std::endl;
            return false;
        }

        timeout_ = timeout;

        // Build the CP model
        buildModel();

        // Configure backend
        if (numWorkers_ > 0) {
            backend_->setNumWorkers(numWorkers_);
        }

        if (verbose_) {
            std::cout << "[CPSolver] Solving with " << backend_->name()
                      << " (timeout: " << timeout_ << "s)" << std::endl;
        }

        // Solve
        CPStatus status = backend_->solve(timeout_);

        if (verbose_) {
            std::cout << "[CPSolver] Status: " << toString(status)
                      << ", Time: " << backend_->getSolveTime() << "s" << std::endl;
        }

        if (status == CPStatus::Optimal || status == CPStatus::Feasible) {
            objectiveValue_ = backend_->getObjectiveValue();
            extractSolution();
            return true;
        }

        return false;
    }

    Solution* getSolution() const override { return solution_; }

    double getObjectiveValue() const override { return objectiveValue_; }

    bool isOptimal() const override {
        return std::abs(backend_->getGap()) < 1e-6;
    }

    std::string getStats() const override {
        std::ostringstream ss;
        ss << "Backend: " << backend_->name() << "\n"
           << "Time: " << backend_->getSolveTime() << "s\n"
           << "Branches: " << backend_->getNumBranches() << "\n"
           << "Failures: " << backend_->getNumFailures() << "\n"
           << "Objective: " << backend_->getObjectiveValue() << "\n"
           << "Bound: " << backend_->getObjectiveBound() << "\n"
           << "Gap: " << (backend_->getGap() * 100) << "%";
        return ss.str();
    }

    // ========== CP-specific configuration ==========

    void setVerbose(bool verbose) { verbose_ = verbose; }
    void setNumWorkers(int workers) { numWorkers_ = workers; }

    /**
     * @brief Add a custom CP constraint generator
     */
    void addGenerator(std::unique_ptr<ICPConstraintGenerator> generator) {
        customGenerators_.push_back(std::move(generator));
    }

    /**
     * @brief Get the backend for advanced usage
     */
    ICPBackend& getBackend() { return *backend_; }
    const ICPBackend& getBackend() const { return *backend_; }

private:
    Problem* problem_;
    Solution* solution_;
    double objectiveValue_;
    double timeout_;
    int numWorkers_;
    bool verbose_;

    std::unique_ptr<ICPBackend> backend_;
    std::vector<std::unique_ptr<ICPConstraintGenerator>> customGenerators_;

    void buildModel() {
        backend_->clear();

        // Collect all active generators
        std::vector<ICPConstraintGenerator*> generators;
        for (auto& gen : customGenerators_) {
            generators.push_back(gen.get());
        }

        // TODO: Also get generators from PluginRegistry based on problem attributes

        // Sort by priority
        std::sort(generators.begin(), generators.end(),
                  [](ICPConstraintGenerator* a, ICPConstraintGenerator* b) {
                      return a->priority() < b->priority();
                  });

        if (verbose_) {
            std::cout << "[CPSolver] Building model with " << generators.size()
                      << " generators" << std::endl;
        }

        // Add variables
        for (auto* gen : generators) {
            if (verbose_) {
                std::cout << "  - Adding variables: " << gen->name() << std::endl;
            }
            gen->addVariables(*backend_, *problem_);
        }

        // Add constraints
        for (auto* gen : generators) {
            if (verbose_) {
                std::cout << "  - Adding constraints: " << gen->name() << std::endl;
            }
            gen->addConstraints(*backend_, *problem_);
        }

        // Build objective
        LinearExpr objective;
        for (auto* gen : generators) {
            gen->addObjectiveTerms(*backend_, *problem_, objective);
        }

        if (!objective.isEmpty()) {
            backend_->minimize(objective);
        }
    }

    void extractSolution() {
        // Create a new solution
        if (solution_) {
            delete solution_;
        }
        solution_ = new Solution(problem_);

        // Let generators extract their solution data
        for (auto& gen : customGenerators_) {
            gen->extractSolution(*backend_, *problem_);
        }

        // TODO: Extract tour information from CP variables
    }
};

} // namespace cp
} // namespace routing
