//
// Created by ali on 5/2/19.
//

#ifndef HYBRID_CONFIGURATIONS_HPP
#define HYBRID_CONFIGURATIONS_HPP


namespace Configuration{
    enum DestructionPolicy{
            RANDOM,
            SEQUENTIAL,
            WORST
    };

        const double precision = 10.0;

        const int destruction_steps = 3;

        const DestructionPolicy destructionPolicy = SEQUENTIAL;
        const DestructionPolicy destructionPolicyGenerator = RANDOM;

        const double epsilon = 1e-6;

}





#endif //HYBRID_CONFIGURATIONS_HPP
