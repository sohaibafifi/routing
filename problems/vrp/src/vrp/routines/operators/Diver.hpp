//
// Created by Sohaib Lafifi on 04/01/2023.
//

#pragma once

#include <core/routines/operators/Diver.hpp>
#include "Constructor.hpp"
#include "../../models/Solution.hpp"

namespace vrp {
    namespace routines {
        struct Diver : routing::Diver {
            bool dive(routing::models::Solution *solution) override{
                Constructor constructor;
                return constructor.bestInsertion(solution, dynamic_cast<vrp::models::Solution* >(solution)->notserved);
            }
        };
    }
}