// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "attributes.hpp"
#include <utility>
#include <iostream>

namespace routing {
    struct Model {
        unsigned getID() const {
            return id;
        }

        void setID(const unsigned &p_id) {
            id = p_id;
        }

        std::string name;

        virtual ~Model() = default;

        inline bool operator==(const Model &other) {
            return other.getID() == this->getID();
        }

        inline bool operator!=(const Model &other) {
            return other.getID() != this->getID();
        }

    private :
        unsigned int id;
    };

}
