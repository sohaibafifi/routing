
#pragma once

#include "../../../data/attributes/RemoveCost.hpp"


namespace CVRP {
    class RemoveCost : public routing::RemoveCost {
        private:
            routing::Duration delta;
            bool possible;
    public:
        bool operator>(const RemoveCost &rhs) const;

        routing::Duration getDelta() const;

        void setDelta(routing::Duration delta);

        RemoveCost(routing::Duration delta);

        RemoveCost(const RemoveCost &cost);



    };

}


