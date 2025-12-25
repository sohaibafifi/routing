//
// Created by ali on 5/9/19.
//

#ifndef HYBRID_DIVER_HPP
#define HYBRID_DIVER_HPP

#include <map>
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/solvers/MIPSolverPlugin/callbacks.hpp"

namespace routing {

    class Diver {

    public:
        virtual bool dive(routing::Solution *solution) = 0;

    };

    class dummyDiver : public Diver {
    public:
        virtual bool dive(routing::Solution *solution) { return false; }
    };
}


#endif //HYBRID_DIVER_HPP
