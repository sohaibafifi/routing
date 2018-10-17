#include "IDCH.hpp"

bool routing::IDCH::look(routing::models::Solution *solution)
{
    int itermax = solution->getProblem()->clients.size();
    int iter = 1;
    routing::models::Solution *best = solution->clone();
    bool improved = false;
    double bestCost = solution->getCost();
    unsigned dmax = solution->getProblem()->clients.size() / solution->getProblem()->vehicles.size();

    while (iter < itermax) {
        iter++;
        destructor->destruct(solution, dmax);
        if(constructor->bestInsertion(solution) && solution->getCost() < bestCost - 1e-9)
        {
            bestCost = solution->getCost();
            iter = 1;
            best = solution->clone();
            improved = true;
        }

    }
    solution = best->clone();
    return improved;
}
