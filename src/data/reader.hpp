#pragma once

#include "problem.hpp"
namespace routing {

    class Reader{
            friend class Problem;
        public :
            virtual Problem * readFile(std::string filepath) = 0;

    };

}
