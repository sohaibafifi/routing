#pragma once
#include <sstream>

#include "Problem.hpp"

namespace CVRP
{
    class Reader
            : public routing::Reader
    {
        public:
            CVRP::Problem *readFile(std::string filepath);
            virtual ~Reader(){}
    };


}
