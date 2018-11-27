#pragma once
#include "../../models/Solution.hpp"
#include "../../../../routines/operators/Decoder.hpp"

namespace CVRP {

    class Decoder : public routing::Decoder
    {
        public:
            virtual bool decode(const std::vector<routing::models::Client *> & sequence, routing::models::Solution * solution) override;
    };

}
