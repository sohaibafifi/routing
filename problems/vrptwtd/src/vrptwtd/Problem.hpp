// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <cvrptw/Problem.hpp>

namespace vrptwtd {

    class Problem : public cvrptw::Problem {
    public :
        Problem() : cvrptw::Problem(){  };

        explicit Problem(const cvrptw::Problem & p_problem) : cvrptw::Problem(p_problem){  };
        routing::callback::HeuristicCallback *setHeuristicCallback() override;

        routing::callback::UserCutCallback *setUserCutCallback() override;

        routing::callback::LazyConstraintCallback *setLazyConstraintCallback() override;

        routing::callback::InformationCallback *setInformationCallback() override;

        routing::callback::IncumbentCallback *setIncumbentCallback() override;

    protected:
        virtual void addSynchronisationConstraints();
        virtual void addConstraints() override{
            cvrptw::Problem::addConstraints();
            addSynchronisationConstraints();
        }

    };


}


