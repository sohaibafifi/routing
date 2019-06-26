//
// Created by ali on 6/26/19.
//

#ifndef HYBRID_IDCH_TSPTWH_HPP
#define HYBRID_IDCH_TSPTWH_HPP

#include "Neighborhood.hpp"

namespace routing {

    class IDCH_TSPTWH: public Neighborhood {
        public:
            IDCH_TSPTWH(){

            }
            virtual bool look(models::Solution *solution);

        private:

    };

}

#endif //HYBRID_IDCH_TSPTWH_HPP
