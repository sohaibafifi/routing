#pragma once
#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
#include "../operators/Constructor.hpp"
namespace routing {
    class Move : public Neighborhood{
        public :
            Move(Constructor * p_constructor ):
                constructor(p_constructor){

            }
            virtual bool look(models::Solution *solution);
        private:
            Constructor * constructor;
    };
}
