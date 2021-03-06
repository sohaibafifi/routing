// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/attributes.hpp"
#include "ServiceQuery.hpp"

namespace routing {

    namespace attributes {


        struct Service {
            Service(const Duration &p_start) : start(p_start) {}

            SolutionValue<Duration> start;

            Duration getStart() const { return this->start.getValue(); }

            void setStart(Duration start) { this->start.setValue(start); }
        };

    }
}
