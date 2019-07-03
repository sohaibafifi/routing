//
// Created by ali on 6/18/19.
//

#include "Swap.hpp"
#include <algorithm>



/*

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
*/


bool routing::Swap::look(routing::models::Solution *solution)
{

    //create a structure that contains the tour number and all its client positions
    std::vector<std::pair<int,int>> mappingTourPosition;
    routing::models::Solution *best = solution->clone();
    routing::models::Solution *copy = solution->clone();

    //print(solution);
    bool improved = false;

    //init the structure
    for (int i = 0; i < solution->getNbTour(); ++i) {
        //avoid empty tours
        if(solution->getTour(i)->getNbClient() == 0 ) continue;

        for (int j = 0; j < solution->getTour(i)->getNbClient(); ++j) {
            mappingTourPosition.push_back(std::make_pair(i,j));
        }

    }
    //shuffle tours
    std::random_shuffle(mappingTourPosition.begin(), mappingTourPosition.end());

    int pairIndex = 0;

    while (pairIndex < mappingTourPosition.size()){
        if(doSwap(copy,mappingTourPosition[pairIndex])){
            //Check for solution improvement
            if(copy->getCost() < best->getCost() -  1e-9 ){
                solution->update(copy);
                improved = true;
                return improved;
            }else
            {
                //backup solution to its original form
                copy = best->clone();
                pairIndex++;
            }
        }
        else{
            copy = best->clone();
            pairIndex++;
        }
    }

    return false;

}



bool routing::Swap::doSwap(routing::models::Solution *solution, std::pair<int, int> tourPosition)
{
    throw new std::logic_error("Not implemented");
}

