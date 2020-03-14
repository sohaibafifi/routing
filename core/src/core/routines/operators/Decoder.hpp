// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../../data/models/Solution.hpp"

namespace routing {

    class Decoder {
    public:
        virtual bool decode(const std::vector<models::Client *> &sequence, models::Solution *solution) = 0;
    };

    class dummyDecoder : public Decoder {
    public:
        virtual bool decode(const std::vector<models::Client *> &sequence, models::Solution *solution) { return false; }
    };
}
