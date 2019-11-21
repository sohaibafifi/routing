//
// Created by ali on 5/9/19.
//

#ifndef HYBRID_DIVER_HPP
#define HYBRID_DIVER_HPP

#include <map>
#include "../../data/models/Solution.hpp"
#include "core/routines/callbacks.hpp"

namespace routing {

    class Diver {

    public:
        virtual bool dive(routing::models::Solution *solution) = 0;

    };

    class dummyDiver : public Diver {
    public:
        virtual bool dive(routing::models::Solution *solution) { return false; }
    };
}


#endif //HYBRID_DIVER_HPP
