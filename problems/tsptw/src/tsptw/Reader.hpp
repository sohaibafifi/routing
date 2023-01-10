// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once


#include <cvrptw/Reader.hpp>

namespace tsptw {
    class Reader : public cvrptw::Reader {
    public :
        routing::Problem *readFile(const std::string & filepath) override;
    };
}
