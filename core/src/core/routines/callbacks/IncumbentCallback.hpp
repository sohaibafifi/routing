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
                std::string source = "";
                if(getSolutionSource() == this->NodeSolution)
                    source = "NodeSolution";
                if(getSolutionSource() == this->HeuristicSolution)
                    source = "HeuristicSolution";
                if(getSolutionSource() == this->UserSolution)
                    source = "UserSolution";
                if(getSolutionSource() == this->MIPStartSolution)
                    source = "MIPStartSolution";

                getEnv().out() << "Incumbent found of value " << getObjValue()  << " using "  << source << std::endl;
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
