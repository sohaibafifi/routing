//
// Created by ali on 3/28/19.
//

#include "Destructor.h"

void CVRPTW::Destructor::destruct(routing::models::Solution *solution, unsigned long n)
{
    std::cout << "destructor has been called " << std::endl;
    CVRP::Destructor::destruct(solution,n);

}