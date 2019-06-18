//
// Created by ali on 5/27/19.
//

#pragma once
#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
namespace routing {
    class Shift : public Neighborhood{
    public :
        Shift() {}
        virtual bool look(models::Solution *solution);

        virtual bool doShift(routing::models::Solution* solution, std::pair<int,int> tourPosition);
    };
}
