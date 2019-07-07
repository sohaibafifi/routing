//
// Created by ali on 6/26/19.
//

#include "IDCH_TSPTWH.hpp"
#include "../operators/SequentialDestructionParameters.hpp"
#include "../../mtrand.hpp"
#include "../operators/TSPTWDestructionParameters.hpp"

void print(routing::models::Solution *solution){
    std::cout << "------------------------------"<<std::endl;
    for (int i = 0; i < solution->getNbTour(); ++i) {
        if (solution->getTour(i)->getNbClient() == 0) continue;
        std::cout << "Tour " << i << " : " << std::endl;
        for (int j = 0; j < solution->getTour(i)->getNbClient(); ++j) {
            std::cout << solution->getTour(i)->getClient(j)->getID() << std::setw(3);
        }
        std::cout << std::endl;
    }
}

bool routing::IDCH_TSPTWH::look(routing::models::Solution *solution)
{
    int itermax = solution->getProblem()->clients.size() ;
    int iter = 0;
    routing::models::Solution *best = solution->clone();

    bool improved = false;
    double bestCost = solution->getCost();

    Utilities::MTRand_int32 irand(std::time(nullptr));
    int i = 0;

    unsigned int nonEmptyTour = 0;
    //get number of non empty tour to loop in
    for (int j = 0; j < solution->getNbTour(); ++j) {
        if (solution->getTour(j)->getNbClient() == 0) continue;
        nonEmptyTour++;
    }
    std::vector<bool> run(nonEmptyTour, false);

    unsigned int nbTourToProcess = 0;

    //vector to contain clients removed form tour that should be routed
    //filled in destructor and re-routed in bestInsertion
    std::vector<routing::models::Client*> toRouteClients;
    TSPTWDestructionParameters* parameters = new TSPTWDestructionParameters(0,0,0,toRouteClients);

    while(std::find(run.begin(), run.end(), false) != run.end() && nbTourToProcess < nonEmptyTour) {


        //take a tour index randomly
        do { i = irand() % run.size(); } while (run[i]);
        unsigned long start = 0;
        unsigned long end  = 2;
        toRouteClients.clear();

        if(solution->getTour(i)->getNbClient() == 1){
            run[i] = true;
            continue;
        }

        while (!run[i] && iter < itermax) {
            iter++;

            parameters->setStartPosition(start);
            parameters->setEndPosition(end);
            parameters->setTourIndex(i);
            parameters->setToRoute(toRouteClients);

            destructor->destruct(solution, parameters);


            //shuffle the clients randomly before reinserting them
            std::random_shuffle(parameters->toRoute.begin(), parameters->toRoute.end());

            if (constructor->bestInsertion(solution, parameters) && solution->getCost() < bestCost - 1e-09) {
                bestCost = solution->getCost();
                best = solution->clone();
                improved = true;
                run[i] = true;
            }

            start += end;
            end += end;

            if (start > solution->getTour(i)->getNbClient()) break;
        }


        //update number of tours to process, in case of no improvement i.e: we reached itermax iterations
        //and there are still run[i] at false, we leave the local search
        //in hope either to launch another type of local search or to give hand to cplex to explore another node
        nbTourToProcess++;

    }

    solution->update(best);
    return improved;
}