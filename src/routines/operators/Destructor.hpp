#pragma once
#include "../../data/models/Solution.hpp"
#include "DestructionParameters.hpp"
namespace routing{
    class Destructor
    {
        public:


            virtual void destruct(models::Solution *solution, routing::DestructionParameters* param) = 0;


    };
    class dummyDestructor : public Destructor{
        public:
            virtual void destruct(models::Solution *solution, routing::DestructionParameters* param){}
    };


}
