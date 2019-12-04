//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once

#include <core/data/Problem.hpp>
#include "models/Depot.hpp"

namespace vrp {
    namespace models {
        class Solution;

        class Tour;
    }
    class Initializer : public routing::Initializer {
    public:
        Initializer(routing::Problem *pProblem) : routing::Initializer(pProblem) {}

    private:
        routing::models::Solution *initialSolution() override;

        routing::models::Tour *initialTour(int vehicleID) override;
    };

    class Problem : public routing::Problem {
    public:
        Problem() {}

#ifdef CPLEX
        virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;

        virtual routing::callback::IncumbentCallback *setIncumbentCallback(IloEnv &env) override;

        std::vector<std::vector<IloNumVar> > arcs;
        std::vector<std::vector<IloNumVar> > affectation;
        std::vector<IloNumVar> order;
        std::vector<IloNumVar> consumption;
#endif

        virtual routing::Duration
        getDistance(const routing::models::Client &c1, const routing::models::Client &c2) const override;

        virtual routing::Duration
        getDistance(const routing::models::Client &c1, const routing::models::Depot &d) const override;

        std::vector<std::vector<routing::Duration> > distances, distances_to_depots;

        virtual models::Depot *getDepot() {
            return dynamic_cast<models::Depot *>(depots[0]);
        }


        virtual void setDepot(models::Depot *depot) {
            depots.clear();
            depots.push_back(depot);
        }

        routing::Initializer *initializer() override {
            return new Initializer(this);
        }

    protected:
#ifdef CPLEX

        virtual void addVariables() override;

        virtual void addConstraints() override;

        virtual void addObjective() override;


        virtual void addTotalDistanceObjective();

        virtual void addAffectationConstraints();

        virtual void addRoutingConstraints();

        virtual void addSequenceConstraints();

        virtual void addCapacityConstraints();
#endif
    };

}



