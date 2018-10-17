#pragma once
#include "../attributes.hpp"
namespace routing {
    typedef double Profit;
    namespace attributes {


        /**
 * @brief a node with a profit property
 *
 */
        struct Profiter
        {
                Profiter(const Profit & p_profit):profit(p_profit){}
                EntityData<Profit> profit;
                double getProfit() const { return this->profit.getValue();}
        };

    }
}
