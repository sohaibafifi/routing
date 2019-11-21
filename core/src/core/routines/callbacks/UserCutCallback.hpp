#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcplex/ilocplexi.h>

#pragma GCC diagnostic pop

#include "../../data/Problem.hpp"

namespace routing {
    namespace callback {

        class UserCutCallback
                : public IloCplex::UserCutCallbackI {
        public:
            UserCutCallback(IloEnv env, Problem *_problem)
                    :
                    IloCplex::UserCutCallbackI(env),
                    problem(_problem) {}

            ~UserCutCallback() {}

        protected:
            void main() {}

            IloCplex::CallbackI *duplicateCallback() const {
                throw new std::logic_error("Not implemented");
            }

            Problem *problem;

        };

    }
}