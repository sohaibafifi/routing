//
// Created by ali on 6/26/19.
//

#ifndef HYBRID_IDCH_RANDOM_HPP
#define HYBRID_IDCH_RANDOM_HPP

#include "IDCH.hpp"

namespace routing{
    class IDCH_Random: public IDCH {
        public:
            IDCH_Random(Constructor * p_constructor, Destructor * p_destructor): IDCH(p_constructor,p_destructor){}

            virtual bool look(models::Solution *solution) override;
    };

}


#endif //HYBRID_IDCH_RANDOM_HPP
