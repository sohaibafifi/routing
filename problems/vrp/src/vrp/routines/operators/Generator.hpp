// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <core/routines/operators/Generator.hpp>
#include "../../models/Solution.hpp"

namespace vrp {
    namespace routines {
        class Generator : public routing::Generator {

        public:
            Generator(routing::Problem *pProblem, routing::Constructor *pConstructor, routing::Destructor *pDestructor)
                    : routing::Generator(pProblem, pConstructor, pDestructor) {

            }
        };
    }
}