#pragma once

#include "core/data/Model.hpp"

namespace routing {

    namespace models {

        struct Vehicle : public Model {
            Vehicle(unsigned id) {
                Model::setID(id);
            }
        };
    }
}
