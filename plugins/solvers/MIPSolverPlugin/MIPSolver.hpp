
// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "plugins/solvers/SolverCorePlugin/Solver.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "callbacks.hpp"
#include <ilcplex/ilocplex.h>

namespace routing {

    class MIPSolver : public Solver {
        IloCplex* cplex = nullptr;
    public:

        explicit MIPSolver(routing::Problem *p_problem, std::ostream &os = std::cout);

        virtual void setDefaultConfiguration() override {
            this->configuration = new Configuration();
        };

        virtual bool solve(double timeout = 3600) override;
        virtual void tune(double timeout = 3600);

        virtual ~MIPSolver();

        virtual IloCplex& getCplex() { return *cplex; }
        virtual const IloCplex& getCplex() const { return *cplex; }

        virtual void save(std::ofstream &output) const override;

    private:
        void extractSolutionFromArcs();
    };

    inline MIPSolver::MIPSolver(routing::Problem *p_problem, std::ostream &os)
        : Solver(p_problem, os) {
        this->cplex = &this->problem->generateModel();

        this->setDefaultConfiguration();
        std::vector<IloCplex::CallbackI *> callbacks = std::vector<IloCplex::CallbackI *>();
        callbacks.push_back(this->problem->setHeuristicCallback());
        callbacks.push_back(this->problem->setUserCutCallback());
        callbacks.push_back(this->problem->setIncumbentCallback());
        callbacks.push_back(this->problem->setInformationCallback());
        callbacks.push_back(this->problem->setLazyConstraintCallback());
        for (auto callback: callbacks) {
            if (callback != nullptr)
                this->cplex->use(callback);
        }
    }

    inline bool MIPSolver::solve(double timeout) {
        // this->cplex.setParam(this->cplex.Threads, 1);
        this->cplex->setParam(IloCplex::Param::MIP::Display, 4);
        this->cplex->setParam(IloCplex::Param::MultiObjective::Display, 2);
        this->cplex->setParam(IloCplex::Param::TimeLimit, timeout);
        //this->cplex.setParam(IloCplex::Param::Preprocessing::Reduce, 0);
        cplex->resetTime();
        bool solved = this->cplex->solve() != 0;

        // Check status before accessing solution values
        auto status = this->cplex->getStatus();
        bool hasSolution = (status == IloAlgorithm::Optimal ||
                           status == IloAlgorithm::Feasible);

        this->os << this->problem->getName() << "\t" << status;
        if (hasSolution) {
            this->os << "\t" << this->cplex->getObjValue()
                     << "\t" << this->cplex->getBestObjValue()
                     << "\t" << this->cplex->getMIPRelativeGap();
        } else {
            this->os << "\tN/A\tN/A\tN/A";
        }
        this->os << "\t" << this->cplex->getTime() << std::endl;

        // Extract solution from CPLEX arc variables
        this->solution = this->problem->initializer()->initialSolution();
        if (hasSolution && !this->problem->arcs.empty()) {
            extractSolutionFromArcs();
        }

        return hasSolution;
    }

    inline void MIPSolver::extractSolutionFromArcs() {
        auto* composableProblem = dynamic_cast<Problem*>(this->problem);
        if (!composableProblem) return;

        auto clients = composableProblem->getComposableClients();
        auto vehicles = composableProblem->getComposableVehicles();
        size_t n = clients.size();
        size_t m = vehicles.size();

        if (n == 0 || composableProblem->arcs.empty()) return;

        // Track which clients are visited
        std::vector<bool> visited(n + 1, false);
        visited[0] = true;  // Depot is always "visited"

        unsigned vehicleId = 0;

        // For each potential route starting from depot
        for (size_t startClient = 1; startClient <= n && vehicleId < m; ++startClient) {
            // Check if there's an arc from depot (0) to this client
            try {
                double arcValue = this->cplex->getValue(composableProblem->arcs[0][startClient]);
                if (arcValue > 0.5 && !visited[startClient]) {
                    // Start a new tour
                    auto* tour = dynamic_cast<Tour*>(composableProblem->initializer()->initialTour(vehicleId));
                    if (!tour) continue;

                    // Follow the route
                    size_t current = startClient;
                    while (current != 0 && !visited[current]) {
                        visited[current] = true;
                        tour->_pushClient(clients[current - 1]);  // clients are 0-indexed

                        // Find next node
                        size_t next = 0;
                        for (size_t j = 0; j <= n; ++j) {
                            if (j != current) {
                                double nextArcValue = this->cplex->getValue(composableProblem->arcs[current][j]);
                                if (nextArcValue > 0.5) {
                                    next = j;
                                    break;
                                }
                            }
                        }
                        current = next;
                    }

                    if (tour->getNbClient() > 0) {
                        tour->update();
                        dynamic_cast<Solution*>(this->solution)->pushTour(tour);
                        vehicleId++;
                    } else {
                        delete tour;
                    }
                }
            } catch (IloException&) {
                // Variable might not exist or be accessible
                continue;
            }
        }

        if (this->solution) {
            dynamic_cast<Solution*>(this->solution)->update();
        }
    }

    inline void MIPSolver::tune(double timeout) {
        IloCplex::ParameterSet paramSet = cplex->getParameterSet();

        // this->cplex.setParam(this->cplex.Threads, 1);
        paramSet.setParam(IloCplex::Param::MIP::Display, 4);
        paramSet.setParam(IloCplex::Param::MultiObjective::Display, 2);
        paramSet.setParam(IloCplex::Param::TimeLimit, timeout);
        paramSet.setParam(IloCplex::Param::Tune::TimeLimit, timeout / 5.0);
        cplex->setParameterSet(paramSet);

        IloInt tunestat = this->cplex->tuneParam(paramSet);
        if (tunestat == IloCplex::TuningComplete)
            std::cout << "Tuning complete." << std::endl;
        else if (tunestat == IloCplex::TuningAbort)
            std::cout << "Tuning abort." << std::endl;
        else if (tunestat == IloCplex::TuningTimeLim)
            std::cout << "Tuning time limit." << std::endl;
        else
            std::cout << "Tuning status unknown." << std::endl;
    }

    inline MIPSolver::~MIPSolver() {
        // CPLEX object belongs to the Problem instance.
    }

    inline void MIPSolver::save(std::ofstream &output) const {
        auto status = getCplex().getStatus();
        bool hasSolution = (status == IloAlgorithm::Optimal ||
                           status == IloAlgorithm::Feasible);

        output << this->getProblem()->getName() << "\t" << status;
        if (hasSolution) {
            output << "\t" << getCplex().getObjValue()
                   << "\t" << getCplex().getBestObjValue()
                   << "\t" << getCplex().getMIPRelativeGap();
        } else {
            output << "\tN/A\tN/A\tN/A";
        }
        output << "\t" << getCplex().getTime() << std::endl;
        output.close();
    }

} // namespace routing
