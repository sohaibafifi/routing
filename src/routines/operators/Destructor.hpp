#pragma once
#include "../../data/models/Solution.hpp"
namespace routing{
    class Destructor
    {
        public:
            virtual void destruct(models::Solution *solution, unsigned long n) = 0;
    };
    class dummyDestructor : public Destructor{
        public:
            virtual void destruct(models::Solution *solution, unsigned long n){}
    };
}
