//
// Created by ali on 3/28/19.
//

#include "Destructor.hpp"

void CVRPTW::Destructor::destruct(routing::models::Solution *solution, unsigned long n)
{
    std::cout << "\n destructor has been called " << std::endl;
    CVRP::Destructor::destruct(solution,n);

}