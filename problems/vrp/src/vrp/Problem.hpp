// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <core/data/Problem.hpp>
#include "models/Depot.hpp"

namespace vrp {
    class Initializer : public routing::Initializer {
    public:
        explicit Initializer(routing::Problem *pProblem) : routing::Initializer(pProblem) {}

        routing::models::Solution *initialSolution() override;

        routing::models::Tour *initialTour(int vehicleID) override;
    };

    class Memory : public routing::Memory {
    public:
        static routing::Memory *get();

    };

    class Problem : public routing::Problem {
    public:
#ifdef CPLEX_FOUND
        virtual routing::callback::HeuristicCallback *setHeuristicCallback() override;

        virtual routing::callback::IncumbentCallback *setIncumbentCallback() override;

        std::vector<std::vector<IloNumVar> > arcs;
        std::vector<std::vector<IloNumVar> > affectation;
        std::vector<IloNumVar> order;

#endif
        routing::Duration
        getDistance(const routing::models::Client &c1, const routing::models::Client &c2) const override;

        routing::Duration
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

        routing::Memory *getMemory() override;

    protected:
#ifdef CPLEX_FOUND
        virtual void addVariables() override;

        virtual void addConstraints() override;

        virtual void addObjective() override;

        virtual void addAffectationConstraints();

        virtual void addRoutingConstraints();

        virtual void addSequenceConstraints();
#endif
    };

}



