#pragma once
#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
#include "../operators/Destructor.hpp"
#include "../operators/Constructor.hpp"
#include "../../Configurations.hpp"
#include "../operators/RandomDestructionParameters.hpp"
#include "../operators/SequentialDestructionParameters.hpp"
namespace routing {
    class IDCH : public Neighborhood{
        public :
            IDCH(Constructor * p_constructor, Destructor * p_destructor):
                constructor(p_constructor), destructor(p_destructor){

            }
            virtual bool look(models::Solution *solution);

        protected:
            Constructor * constructor;
            Destructor * destructor;
    };

}
