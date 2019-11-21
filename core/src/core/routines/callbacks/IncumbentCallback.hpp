#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcplex/ilocplexi.h>

#pragma GCC diagnostic pop

#include "../../data/Problem.hpp"
#include "../../data/models/Solution.hpp"

namespace routing {
    namespace callback {
        /*!
         * @brief callback trigger every time a new incumbent is found
         */
        class IncumbentCallback
                : public IloCplex::IncumbentCallbackI {
        public:
            IncumbentCallback(IloEnv env, Problem *_problem)
                    :
                    IloCplex::IncumbentCallbackI(env),
                    problem(_problem) {}

            ~IncumbentCallback() {}

        protected:
            Problem *problem;

            virtual void main() {
                getEnv().out() << "Incumbent found of value " << getObjValue() << std::endl;
                extractIncumbentSolution();
                if (solution != nullptr)
                    solution->print(getEnv().out());
            }

            models::Solution *solution = nullptr;

            IloCplex::CallbackI *duplicateCallback() const {
                throw new std::logic_error("Not implemented");
            }

            virtual void extractIncumbentSolution() {}
        };
    }
}
