//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once

#include <core/data/Problem.hpp>

namespace vrp {
    class Problem : public routing::Problem {
    public:
        virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;

        virtual routing::callback::IncumbentCallback *setIncumbentCallback(IloEnv &env) override;

        virtual routing::callback::UserCutCallback *setUserCutCallback(IloEnv &env) override;

        virtual routing::callback::LazyConstraintCallback *setLazyConstraintCallback(IloEnv &env) override;

        virtual routing::callback::InformationCallback *setInformationCallback(IloEnv &env) override;

        virtual routing::Duration
        getDistance(const routing::models::Client &c1, const routing::models::Client &c2) const override;

        virtual routing::Duration
        getDistance(const routing::models::Client &c1, const routing::models::Depot &d) const override;

        std::vector<std::vector<IloNumVar> > arcs;
        std::vector<std::vector<IloNumVar> > affectation;
        std::vector<IloNumVar> order;
        std::vector<std::vector<routing::Duration> > distances, distances_to_depots;

        routing::models::Depot *getDepot();

        void setDepot(routing::models::Depot *);

    protected:
        virtual void addVariables() override;

        virtual void addConstraints() override;

        virtual void addObjective() override;

        virtual void addTotalDistanceObjective();

        virtual void addAffectationConstraints();

        virtual void addRoutingConstraints();

        virtual void addSequenceConstraints();
    };

}



