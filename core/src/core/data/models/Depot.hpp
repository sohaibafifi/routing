#pragma once

#include "core/data/Model.hpp"

namespace routing {

    namespace models {

        struct Depot : public Model {
            explicit Depot(unsigned id) {
                Model::setID(id);
            }
        };
    }
}
