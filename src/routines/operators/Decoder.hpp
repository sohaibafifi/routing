#pragma once
#include "../../data/models/Solution.hpp"
namespace routing {

    class Decoder
    {
        public:
            virtual bool decode(const std::vector<models::Client *> & sequence, models::Solution * solution) = 0;
    };

    class dummyDecoder :  public Decoder
    {
        public:
            virtual bool  decode(const std::vector<models::Client *> & sequence, models::Solution * solution) {return false;}
    };
}
