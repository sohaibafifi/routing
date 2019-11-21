#pragma once

#include "core/data/Model.hpp"

namespace routing {

    namespace models {

        struct Client : public Model {
            explicit Client(unsigned id) {
                Model::setID(id);
            }
        };
    }
}
