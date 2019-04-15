//
// Created by ali on 4/14/19.
//

#ifndef HYBRID_CHECKER_HPP
#define HYBRID_CHECKER_HPP

#include "../data/models/Solution.hpp"

namespace routing{
    class Checker {
        public:
            virtual bool check(routing::models::Solution* solution) = 0;

    };

}


#endif //HYBRID_CHECKER_HPP
