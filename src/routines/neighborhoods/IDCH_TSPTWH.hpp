//
// Created by ali on 6/26/19.
//

#ifndef HYBRID_IDCH_TSPTWH_HPP
#define HYBRID_IDCH_TSPTWH_HPP

#include "Neighborhood.hpp"
#include "../../problems/cvrptw/routines/operators/Constructor_TSPTWH.hpp"
#include "../operators/Destructor.hpp"
#include "../../problems/cvrptw/routines/operators/Destructor_TSPTWH.hpp"

namespace routing {

    class IDCH_TSPTWH: public Neighborhood {
        public:
            IDCH_TSPTWH(CVRPTW::Constructor_TSPTWH* p_constructor, CVRPTW::Destructor_TSPTWH* p_destructor):
                        constructor(p_constructor),destructor(p_destructor){}

            virtual bool look(models::Solution *solution);

        private:
            CVRPTW::Constructor_TSPTWH* constructor;
            CVRPTW::Destructor_TSPTWH* destructor;
    };

}

#endif //HYBRID_IDCH_TSPTWH_HPP
