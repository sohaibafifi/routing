// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrptw/Problem.hpp>

namespace cvrpstw {

    class Problem : public cvrptw::Problem {
    public:

        Problem() : cvrptw::Problem(), waitPenalty(1), delayPenalty(1){  };
        explicit Problem(const cvrptw::Problem & p_problem) : cvrptw::Problem(p_problem){  };


        std::vector<IloNumVar> wait;
        std::vector<IloNumVar> delay;
        IloObjective obj_penality;


        double waitPenalty, delayPenalty;

        double getWaitPenalty() const;

        void setWaitPenalty(double waitPenalty);

        double getDelayPenalty() const;

        void setDelayPenalty(double delayPenalty);

        routing::callback::HeuristicCallback *setHeuristicCallback() override;

        routing::callback::UserCutCallback *setUserCutCallback() override;

        routing::callback::LazyConstraintCallback *setLazyConstraintCallback() override;


        routing::callback::IncumbentCallback *setIncumbentCallback() override;

    protected :


        virtual void addVariables() override;

        virtual void addObjective() override;


    };


}


