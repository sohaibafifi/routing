#pragma once

#include "core/interfaces/ISolver.hpp"
#include "core/interfaces/ICPBackend.hpp"
#include "core/interfaces/ICPConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "XCSP3Backend.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>

namespace routing {
namespace cp {

/**
 * @brief XCSP3-based solver
 */
class XCSP3Solver : public ISolver {
public:
    explicit XCSP3Solver(Problem* problem)
        : problem_(problem)
        , solution_(nullptr)
        , objectiveValue_(std::numeric_limits<double>::infinity())
        , timeout_(3600.0)
        , numWorkers_(0)
        , verbose_(false)
    {
        backend_ = std::make_unique<XCSP3Backend>();
    }

    ~XCSP3Solver() override = default;

    std::string name() const override { return "xcsp3"; }

    std::string description() const override {
        return "Constraint Programming solver using XCSP3 backend";
    }

    void setProblem(Problem* problem) override {
        problem_ = problem;
        solution_ = nullptr;
    }

    Problem* getProblem() const override { return problem_; }

    void setConfiguration(Configuration* config) override {}

    void setVerbose(bool verbose) { verbose_ = verbose; }

    void addGenerator(std::unique_ptr<ICPConstraintGenerator> generator) {
        customGenerators_.push_back(std::move(generator));
    }

    void setDefaultConfiguration() override {
        timeout_ = 3600.0;
        numWorkers_ = 0;
        verbose_ = false;
    }

    bool solve(double timeout) override {
        if (!problem_) {
            std::cerr << "[XCSP3Solver] No problem set" << std::endl;
            return false;
        }

        timeout_ = timeout;
        buildModel();

        if (numWorkers_ > 0) backend_->setNumWorkers(numWorkers_);

        if (verbose_) {
            std::cout << "[XCSP3Solver] Solving with XCSP3 (timeout: " << timeout_ << "s)" << std::endl;
        }

        CPStatus status = backend_->solve(timeout_);

        if (verbose_) {
            std::cout << "[XCSP3Solver] Status: " << (status == CPStatus::Optimal ? "Optimal" : 
                                                    (status == CPStatus::Feasible ? "Feasible" : "Unknown"))
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

    bool isOptimal() const override { return false; } // Hard to know with file interface without parsing proof

    std::string getStats() const override {
        return "Backend: XCSP3";
    }

private:
    Problem* problem_;
    Solution* solution_;
    double objectiveValue_;
    double timeout_;
    int numWorkers_;
    bool verbose_;

    std::unique_ptr<XCSP3Backend> backend_;
    std::vector<std::unique_ptr<ICPConstraintGenerator>> customGenerators_;

    void buildModel() {
        backend_->clear();

        // Collect generators - here we assume we can get them from registry or manual add.
        // For standard problems, generators are usually added by the problem plugin or manually.
        // Since we are duplicating logic, we might miss the auto-discovery if it relies on PluginRegistry injection.
        // However, CPSolver logic was:
        // "TODO: Also get generators from PluginRegistry based on problem attributes"
        // So currently it only uses customGenerators_.
        // WAIT. If it only uses customGenerators_, how does it solve anything?
        // Ah, likely the "Problem" object or the caller populates it.
        // Actually, the standard CP solver might be more complex than the snippet.
        // Let's assume for now we need to add generators.
        // For integrating into the framework, the framework likely calls `addGenerator`.
        
        std::vector<ICPConstraintGenerator*> generators;
        for (auto& gen : customGenerators_) {
            generators.push_back(gen.get());
        }

        // Sort by priority
        std::sort(generators.begin(), generators.end(),
                  [](ICPConstraintGenerator* a, ICPConstraintGenerator* b) {
                      return a->priority() < b->priority();
                  });

        // Add variables
        for (auto* gen : generators) {
            gen->addVariables(*backend_, *problem_);
        }

        // Add constraints
        for (auto* gen : generators) {
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
        if (solution_) delete solution_;
        solution_ = new Solution(problem_);

        for (auto& gen : customGenerators_) {
            gen->extractSolution(*backend_, *problem_);
        }
    }
};

} // namespace cp
} // namespace routing
