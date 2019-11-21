#pragma once

#include "Problem.hpp"

namespace routing {

    class Reader {
    public :
        virtual Problem *readFile(std::string filepath) = 0;

    };

}
