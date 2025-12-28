// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/ISolver.hpp"
#include "core/interfaces/IMIPBackend.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "CPLEXMIPBackend.hpp"
#include "HiGHSMIPBackend.hpp"

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>

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
     * @brief Build the MIP model - override in subclasses for custom models
     */
    virtual void buildModel() {
        backend_->clear();

        // Default implementation: build a standard arc-based VRP model
        // Subclasses can override this to create problem-specific models

        auto clients = problem_->getComposableClients();
        auto vehicles = problem_->getComposableVehicles();
        size_t n = clients.size();
        size_t m = vehicles.size();

        if (n == 0 || m == 0) return;

        // Create arc variables: x[i][j] = 1 if arc (i,j) is used
        // Index 0 is depot, indices 1..n are clients
        arcVars_.resize(n + 1);
        for (size_t i = 0; i <= n; ++i) {
            arcVars_[i].resize(n + 1);
            for (size_t j = 0; j <= n; ++j) {
                if (i != j) {
                    std::string varName = "x_" + std::to_string(i) + "_" + std::to_string(j);
                    arcVars_[i][j] = backend_->newBoolVar(varName);
                }
            }
        }

        // Flow conservation: each client visited exactly once
        for (size_t j = 1; j <= n; ++j) {
            // Sum of incoming arcs = 1
            LinearExpr inFlow;
            for (size_t i = 0; i <= n; ++i) {
                if (i != j) {
                    inFlow.addTerm(arcVars_[i][j]);
                }
            }
            backend_->addEqual(inFlow, 1.0, "in_" + std::to_string(j));

            // Sum of outgoing arcs = 1
            LinearExpr outFlow;
            for (size_t k = 0; k <= n; ++k) {
                if (j != k) {
                    outFlow.addTerm(arcVars_[j][k]);
                }
            }
            backend_->addEqual(outFlow, 1.0, "out_" + std::to_string(j));
        }

        // Depot flow: vehicles leave and return
        LinearExpr depotOut, depotIn;
        for (size_t j = 1; j <= n; ++j) {
            depotOut.addTerm(arcVars_[0][j]);
            depotIn.addTerm(arcVars_[j][0]);
        }
        backend_->addLessEqual(depotOut, static_cast<double>(m), "depot_out");
        backend_->addEqual(depotOut, depotIn, "depot_balance");

        // Subtour elimination using MTZ formulation
        std::vector<mip::IntVar> u(n + 1);
        for (size_t i = 1; i <= n; ++i) {
            u[i] = backend_->newIntVar(1, static_cast<int>(n), "u_" + std::to_string(i));
        }

        for (size_t i = 1; i <= n; ++i) {
            for (size_t j = 1; j <= n; ++j) {
                if (i != j) {
                    // u[i] - u[j] + n * x[i][j] <= n - 1
                    LinearExpr mtz;
                    mtz.addTerm(u[i]);
                    mtz.addTerm(u[j], -1.0);
                    mtz.addTerm(arcVars_[i][j], static_cast<double>(n));
                    backend_->addLessEqual(mtz, static_cast<double>(n - 1),
                        "mtz_" + std::to_string(i) + "_" + std::to_string(j));
                }
            }
        }

        // Objective: minimize total distance
        LinearExpr objective;
        auto* depot = problem_->getDepot();
        for (size_t i = 0; i <= n; ++i) {
            for (size_t j = 0; j <= n; ++j) {
                if (i != j) {
                    double dist = 0.0;
                    if (i == 0 && j == 0) {
                        dist = 0.0;  // Depot to depot
                    } else if (i == 0) {
                        // From depot to client j-1
                        dist = problem_->getDistanceEntity(depot, clients[j-1]);
                    } else if (j == 0) {
                        // From client i-1 to depot
                        dist = problem_->getDistanceEntity(clients[i-1], depot);
                    } else {
                        // Between clients
                        dist = problem_->getDistanceEntity(clients[i-1], clients[j-1]);
                    }
                    objective.addTerm(arcVars_[i][j], dist);
                }
            }
        }
        backend_->minimize(objective);
    }

    /**
     * @brief Extract solution from the MIP model
     */
    virtual void extractSolution() {
        if (solution_) {
            delete solution_;
        }
        solution_ = new Solution(problem_);

        auto clients = problem_->getComposableClients();
        auto vehicles = problem_->getComposableVehicles();
        size_t n = clients.size();
        size_t m = vehicles.size();

        if (n == 0 || arcVars_.empty()) return;

        // Track which clients are visited
        std::vector<bool> visited(n + 1, false);
        visited[0] = true;  // Depot

        unsigned vehicleId = 0;

        // For each potential route starting from depot
        for (size_t startClient = 1; startClient <= n && vehicleId < m; ++startClient) {
            // Check if there's an arc from depot to this client
            if (backend_->getBoolValue(arcVars_[0][startClient]) && !visited[startClient]) {
                // Start a new tour
                auto* tour = new Tour(problem_, vehicleId);

                // Follow the route
                size_t current = startClient;
                while (current != 0 && !visited[current]) {
                    visited[current] = true;
                    tour->_pushClient(clients[current - 1]);

                    // Find next node
                    size_t next = 0;
                    for (size_t j = 0; j <= n; ++j) {
                        if (j != current && backend_->getBoolValue(arcVars_[current][j])) {
                            next = j;
                            break;
                        }
                    }
                    current = next;
                }

                if (tour->getNbClient() > 0) {
                    tour->update();
                    solution_->pushTour(tour);
                    vehicleId++;
                } else {
                    delete tour;
                }
            }
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

    // Arc variables for extraction
    std::vector<std::vector<BoolVar>> arcVars_;
};

} // namespace routing
