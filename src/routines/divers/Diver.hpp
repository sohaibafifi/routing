//
// Created by ali on 5/9/19.
//

#ifndef HYBRID_DIVER_HPP
#define HYBRID_DIVER_HPP

#include <map>
#include "../../data/models/Solution.hpp"

namespace routing {
    //associate for each ClientID (i) the next ClientIDs (j) that can't follow it either
    //because x_(ij) = 0 or there exists a ClientID (k) such that x_(ik) = 1
    typedef std::map<unsigned int, std::vector<unsigned int>> forbiddenPositions;

    class Diver {

        public:
            virtual bool dive(routing::Problem* problem) = 0;
            virtual routing::models::Solution* extractPartialSolution(routing::Problem* problem) = 0;
            virtual routing::forbiddenPositions* extractForbiddenPositions(routing::Problem* problem) = 0;
    };
}



#endif //HYBRID_DIVER_HPP
