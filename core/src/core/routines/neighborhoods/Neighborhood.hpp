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
