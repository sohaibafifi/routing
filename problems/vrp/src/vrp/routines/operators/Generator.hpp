//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <core/routines/operators/Generator.hpp>
#include "../../models/Solution.hpp"

namespace vrp {
    namespace routines {
        class Generator : public routing::Generator {

            virtual models::Solution *initialSolution() {
                return dynamic_cast<models::Solution *>(this->problem->initialSolution());
            };

        public:
            Generator(routing::Problem *pProblem, routing::Constructor *pConstructor, routing::Destructor *pDestructor)
                    : routing::Generator(pProblem, pConstructor, pDestructor) {

            }
        };
    }
}