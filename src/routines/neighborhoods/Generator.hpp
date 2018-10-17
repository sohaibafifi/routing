#pragma once
#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
#include "../operators/Destructor.hpp"
#include "../operators/Constructor.hpp"
namespace routing {
    class Generator{
        public :
            Generator(Constructor * p_constructor, Destructor * p_destructor):
                constructor(p_constructor), destructor(p_destructor){

            }
            virtual bool generate(models::Solution *solution);
        private:
            Constructor * constructor;
            Destructor * destructor;
    };

}
