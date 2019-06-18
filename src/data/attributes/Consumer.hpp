#pragma once
#include "../attributes.hpp"
namespace routing {
    typedef double Demand;
    namespace attributes {

        /**
 * @brief a node with demand
 *
 */
        struct Consumer
        {
                Consumer(const Demand & p_demand):demand(p_demand), consumption(0){}
                Consumer(){}
                EntityData<Demand> demand;
                SolutionValue<Demand> consumption;
                Demand getConsumption() const { return this->consumption.getValue();}
                void setConsumption(Demand consumption) {this->consumption.setValue(consumption);}

                Demand getDemand() const { return this->demand.getValue();}
        };

    }
}
