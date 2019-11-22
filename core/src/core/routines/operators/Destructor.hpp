#pragma once

#include "../../data/models/Solution.hpp"

namespace routing {
    class DestructionParameters {

    };

    class Destructor {
    public:
        routing::DestructionParameters *params;

        Destructor(routing::DestructionParameters *p_params) : params(p_params) {}

        virtual void destruct(models::Solution *solution) = 0;


    };

    class dummyDestructor : public Destructor {
    public:
        virtual void destruct(models::Solution *solution) {}
    };


}
