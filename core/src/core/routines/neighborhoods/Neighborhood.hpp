#pragma once

#include "../../data/models/Solution.hpp"

namespace routing {
    class Neighborhood {
    public :
        virtual bool look(models::Solution *solution) = 0;
    };

}
