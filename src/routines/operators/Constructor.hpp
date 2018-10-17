#pragma once
#include "../../data/models/Solution.hpp"
namespace routing {

    class Constructor
    {
        public:
            virtual bool bestInsertion(models::Solution * solution) = 0;
    };

    class dummyConstructor :  public Constructor
    {
        public:
            virtual bool bestInsertion(models::Solution * solution){return false;}
    };

}
