#include "IDCH.hpp"

bool routing::IDCH::look(routing::models::Solution *solution)
{
/*

    //itermax is equal to the square of number of clients
    int itermax = solution->getProblem()->clients.size() * solution->getProblem()->clients.size();
    int iter = 0;
    routing::models::Solution *best = solution->clone();
    bool improved = false;
    double bestCost = solution->getCost();

    //dmax takes two different values:
    // for a big destruction: it takes nbClient / nbTours
    //for a small destruction: it takes value of DESTRUCTION_STEPS
    switch (Configuration::destructionPolicy){
        case Configuration::DestructionPolicy::RANDOM:{
            RandomDestructionParameters* dmax  = new RandomDestructionParameters(Configuration::destruction_steps);

            while (iter < itermax) {
                iter++;
                if(iter % solution->getProblem()->clients.size()  == 0){
                    dmax->setDmax(solution->getProblem()->clients.size() / solution->getNbTour());
                }else{
                    dmax->setDmax(Configuration::destruction_steps );
                }


                destructor->destruct(solution, dmax);

                if(constructor->bestInsertion(solution) && solution->getCost() < bestCost - 1e-9)
                {
                    bestCost = solution->getCost();
                    iter = 1;
                    best = solution->clone();
                    improved = true;
                }

            }
            break;
        }

        case Configuration::DestructionPolicy::SEQUENTIAL : {
            unsigned long start = 0;
            unsigned long length  = 1;
            SequentialDestructionParameters* parameters = new SequentialDestructionParameters(start,length);
            itermax = solution->getProblem()->clients.size();

            while (iter < itermax) {
                iter++;

                destructor->destruct(solution, parameters);

                if(constructor->bestInsertion(solution) && solution->getCost() < bestCost - 1e-9)
                {
                    bestCost = solution->getCost();
                    length = 0;
                    best = solution->clone();
                    improved = true;
                }

                start += length;
                length++;

            }

        }
    }

    solution = best->clone();
    return improved;
*/
}
