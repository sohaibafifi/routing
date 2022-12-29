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

    protected:
        void addConstraints() override {
            cvrptw::Problem::addConstraints();
            this->addSynchronisationConstraints();
        }

        virtual void addSynchronisationConstraints();

    public:
        routing::callback::HeuristicCallback *setHeuristicCallback() override;

    protected :



    };


}


