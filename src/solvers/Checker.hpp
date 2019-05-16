//
// Created by ali on 4/14/19.
//

#ifndef HYBRID_CHECKER_HPP
#define HYBRID_CHECKER_HPP

#include "../data/models/Solution.hpp"

namespace routing{

    class Checker {
        public:
            Checker(routing::models::Solution* solution, std::ostream& out = std::cout) : solution(solution), out(out){}
            virtual bool check() = 0 ;

            routing::models::Solution* getSolution() const { return this->solution;}
        protected:
            routing::models::Solution* solution;
            std::ostream& out = std::cout;



    };

}

#endif //HYBRID_CHECKER_HPP
