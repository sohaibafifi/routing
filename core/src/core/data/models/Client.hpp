// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/Model.hpp"

namespace routing {

    namespace models {

        struct Client : public Model {
            explicit Client(unsigned id) {
                Model::setID(id);
            }
        };
    }
}
