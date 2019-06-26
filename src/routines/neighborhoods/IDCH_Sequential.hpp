//
// Created by ali on 6/26/19.
//

#ifndef HYBRID_IDCH_SEQUENTIAL_HPP
#define HYBRID_IDCH_SEQUENTIAL_HPP

#include "IDCH.hpp"

namespace routing{
    class IDCH_Sequential : public IDCH {
        public:
            IDCH_Sequential(Constructor* p_constructor, Destructor* p_destructor): IDCH(p_constructor,p_destructor){}

            virtual bool look(routing::models::Solution* solution) override;
    };


}

#endif //HYBRID_IDCH_SEQENTIAL_HPP
