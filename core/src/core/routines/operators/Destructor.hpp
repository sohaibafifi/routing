// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../../data/models/Solution.hpp"

namespace routing {
    class DestructionParameters {
    public :
        static DestructionParameters *getDefault() {
            return new DestructionParameters;
        }
    };

    class Destructor {
    public:
        routing::DestructionParameters *params = nullptr;

        Destructor(routing::DestructionParameters *p_params) : params(p_params) {}
        Destructor* setParams(routing::DestructionParameters *p_params){
            this->params = p_params;
            return this;
        }
        Destructor()  {}

        virtual void destruct(models::Solution *solution) = 0;


    };
}
