//
// Created by ali on 6/26/19.
//

#include "IDCH_Sequential.hpp"


bool routing::IDCH_Sequential::look(routing::models::Solution *solution)
{
    //itermax is equal to the square of number of clients
    int itermax = solution->getProblem()->clients.size() * solution->getProblem()->clients.size();
    int iter = 0;
    routing::models::Solution *best = solution->clone();
    routing::models::Solution *copy = solution->clone();
    bool improved = false;
    double bestCost = solution->getCost();

    unsigned long start = 0;
    unsigned long length  = 1;
    SequentialDestructionParameters* parameters = new SequentialDestructionParameters(start,length);
    itermax = solution->getProblem()->clients.size();

    while (iter < itermax) {
        iter++;

        destructor->destruct(copy, parameters);

        if(constructor->bestInsertion(copy) && copy->getCost() < bestCost - 1e-9)
        {
            bestCost = copy->getCost();
            length = 0;
            best = copy->clone();
            improved = true;
        }

        start += length;
        length++;

    }

    solution->update(best);
    return improved;
}