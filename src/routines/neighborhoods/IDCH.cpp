#include "IDCH.hpp"
#include "../../Configurations.hpp"


bool routing::IDCH::look(routing::models::Solution *solution)
{
    //itermax is equal to the square of number of clients
    int itermax = solution->getProblem()->clients.size() * solution->getProblem()->clients.size();
    int iter = 1;
    routing::models::Solution *best = solution->clone();
    bool improved = false;
    double bestCost = solution->getCost();

    //dmax takes two different values:
    // for a big destruction: it takes nbClient / nbTours
    //for a small destruction: it takes value of DESTRUCTION_STEPS
    unsigned dmax = Configuration::destruction_steps;

    while (iter < itermax) {
        iter++;
        dmax = ( (iter % solution->getProblem()->clients.size()  == 0)? solution->getProblem()->clients.size() / solution->getNbTour() :  Configuration::destruction_steps );

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
