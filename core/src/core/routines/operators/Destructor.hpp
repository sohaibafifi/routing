#pragma once

#include "../../data/models/Solution.hpp"

namespace routing {
    class DestructionParameters {
        static DestructionParameters* getDefault(){
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

    class dummyDestructor : public Destructor {
    public:
        virtual void destruct(models::Solution *solution) {}
    };


}
