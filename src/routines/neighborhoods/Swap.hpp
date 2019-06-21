//
// Created by ali on 6/18/19.
//


#pragma once
#include "../../data/models/Solution.hpp"
#include "Neighborhood.hpp"
namespace routing {
    class Swap : public Neighborhood{
    public :
        Swap() {}
        virtual bool look(models::Solution *solution);

        virtual bool doSwap(routing::models::Solution* solution, std::pair<int,int> tourPosition);
    };
}
