//
// Created by ali on 5/2/19.
//

#ifndef HYBRID_CONFIGURATIONS_HPP
#define HYBRID_CONFIGURATIONS_HPP


namespace Configuration{
    enum DestructionPolicy{
            RANDOM,
            SEQUENTIAL,
            WORST,
            NODST //no destructor is used
    };

        const double precision = 10.0;

        const double maxQuotaDiving  = 0.7;
        const int destruction_steps = 3;

        const DestructionPolicy destructionPolicy = NODST;
        const DestructionPolicy destructionPolicyGenerator = RANDOM;

        const double epsilon = 1e-6;

        const double idch_proba = 0.5;

}





#endif //HYBRID_CONFIGURATIONS_HPP
