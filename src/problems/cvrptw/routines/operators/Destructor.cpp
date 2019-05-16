//
// Created by ali on 3/28/19.
//

#include "Destructor.hpp"

void CVRPTW::Destructor::destruct(routing::models::Solution *solution, unsigned long n)
{
    CVRP::Destructor::destruct(solution,n);
}
