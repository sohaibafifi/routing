#include "Move.hpp"

bool routing::Move::look(routing::models::Solution *solution)
{
    routing::models::Solution *best = solution->clone();
    bool improved = false;
    double bestCost = solution->getCost();
    // in each tour we look for the best remove and try to insert it into the best position
    for (int t = 0; t < solution->getNbTour(); ++t) {
        if(solution->getTour(t)->getNbClient() == 0 ) continue;
        routing::RemoveCost * bestRemove = solution->getTour(t)->evaluateRemove(0);
        unsigned bestRemove_position = 0;
        for (int p = 1; p < solution->getTour(t)->getNbClient(); ++p) {
            routing::RemoveCost* cost  = solution->getTour(t)->evaluateRemove(p);
            if(&bestRemove < &cost){
                bestRemove_position = p;
                bestRemove = cost;
            }
        }
        solution->getTour(t)->removeClient(bestRemove_position);
        // we use for now best insertion to reinsert the client
        // TODO : best insertion should have a version with custom list or only one client
        if(constructor->bestInsertion(solution) && solution->getCost() < bestCost - 1e-9)
        {
            bestCost = solution->getCost();
            best = solution->clone();
            improved = true;
        }

    }

    solution = best->clone();
    return improved;
}
