// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

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
                return new (getEnv()) UserCutCallback(*this);
            }

            Problem *problem;

        };

    }
}
