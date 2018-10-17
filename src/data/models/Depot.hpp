#pragma once
#include "../Model.hpp"
namespace routing {

    namespace models {

        struct Depot: public Model
        {
                Depot(unsigned id) {
                    Model::setID(id);
                }
        };
    }
}
