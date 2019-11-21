#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcplex/ilocplexi.h>

#pragma GCC diagnostic pop

#include "../../data/Problem.hpp"

namespace routing {
    namespace callback {
        class LazyConstraintCallback
                : public IloCplex::LazyConstraintCallbackI {
        public:
            LazyConstraintCallback(IloEnv env, Problem *_problem)
                    :
                    IloCplex::LazyConstraintCallbackI(env),
                    problem(_problem) {}

            ~LazyConstraintCallback() {}

        protected:
            Problem *problem;

            void main() {}

            IloCplex::CallbackI *duplicateCallback() const {
                throw new std::logic_error("Not implemented");
            }
        };
    }
}