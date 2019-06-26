//
// Created by ali on 6/26/19.
//

#include "IDCH_Random.hpp"


bool routing::IDCH_Random::look(routing::models::Solution *solution)
{
    int itermax = solution->getProblem()->clients.size() * solution->getProblem()->clients.size();
    int iter = 0;
    routing::models::Solution *best = solution->clone();
    routing::models::Solution *copy = solution->clone();
    bool improved = false;
    double bestCost = solution->getCost();


    RandomDestructionParameters* dmax  = new RandomDestructionParameters(Configuration::destruction_steps);

    //dmax takes two different values:
    // for a big destruction: it takes nbClient / nbTours
    //for a small destruction: it takes value of DESTRUCTION_STEPS
    while (iter < itermax) {
        iter++;
        if(iter % copy->getProblem()->clients.size()  == 0){
            dmax->setDmax(copy->getProblem()->clients.size() / copy->getNbTour());
        }else{
            dmax->setDmax(Configuration::destruction_steps );
        }


        destructor->destruct(copy, dmax);

        if(constructor->bestInsertion(copy) && copy->getCost() < bestCost - 1e-9)
        {
            bestCost = copy->getCost();
            iter = 1;
            best = copy->clone();
            improved = true;
        }

    }

    solution->update(best);
    return improved;
}