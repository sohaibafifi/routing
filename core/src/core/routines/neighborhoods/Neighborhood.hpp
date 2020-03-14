// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../../data/models/Solution.hpp"

namespace routing {
    class Neighborhood {
    public :
        /**
         *
         * @param solution: the solution to shake
         *                  it returns the best neighbor
         * @return true if solution improved
         */
        virtual bool look(models::Solution *solution) = 0;
    };

}
