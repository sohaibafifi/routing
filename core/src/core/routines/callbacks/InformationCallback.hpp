// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcplex/ilocplexi.h>

#pragma GCC diagnostic pop

#include "../../data/Problem.hpp"
#include "../../data/models/Solution.hpp"

namespace routing {
    namespace callback {

        class InformationCallback
                : public IloCplex::MIPCallbackI {
        public:
            InformationCallback(IloEnv env, Problem *_problem)
                    :
                    IloCplex::MIPCallbackI(env),
                    problem(_problem) {}

            ~InformationCallback() {}

        protected:
            Problem *problem;

            void main()  {
                if (hasIncumbent())
                    getEnv().out() << "I : " << getIncumbentObjValue() << "\t" << getBestObjValue() << "\t"
                                   << getMIPRelativeGap() << "\t" << getDetTime() << std::endl;
                else
                    getEnv().out() << "I : " << '-' << "\t" << getBestObjValue() << "\t" << getMIPRelativeGap() << "\t"
                                   << getDetTime() << std::endl;

            }

            IloCplex::CallbackI *duplicateCallback() const {
                throw new std::logic_error("Not implemented");
            }

            virtual models::Solution *getIncumbentSolution() { return nullptr; }
        };
    }
}
