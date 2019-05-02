#pragma once
#include "../../data/models/Solution.hpp"
namespace routing{
    class Destructor
    {
        public:
            enum DestructionPolicy{
                RANDOM,
                SEQUENTIAL,
                WORST
            };

            virtual void destruct(models::Solution *solution, unsigned long n, DestructionPolicy policy) = 0;


    };
    class dummyDestructor : public Destructor{
        public:
            virtual void destruct(models::Solution *solution, unsigned long n){}
    };


}
