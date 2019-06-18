//
// Created by ali on 3/28/19.
//


#include "HeuristicCallback.hpp"
#include "../operators/Constructor.hpp"
#include "../operators/RandomDestructor.hpp"
#include "../operators/SequentialDestructor.hpp"
#include "../../../../routines/neighborhoods/IDCH.hpp"
#include "../../../../Configurations.hpp"
//#include "../../../../routines/neighborhoods/Shift.hpp"
#include "../operators/Shift.hpp"

routing::callback::HeuristicCallback *CVRPTW::Problem::setHeuristicCallback(IloEnv &env)
{
    std::vector<routing::Neighborhood*> neighborhoods;
    switch(Configuration::destructionPolicy){
        case Configuration::DestructionPolicy::RANDOM:{
            neighborhoods.push_back(new routing::IDCH(new CVRPTW::Constructor, new CVRPTW::RandomDestructor));
            return new HeuristicCallback(env, this,
                                         new routing::Generator(new CVRPTW::Constructor, new CVRPTW::RandomDestructor),
                                         neighborhoods);

        }
        case Configuration::DestructionPolicy::SEQUENTIAL:{
            neighborhoods.push_back(new routing::IDCH(new CVRPTW::Constructor, new CVRPTW::SequentialDestructor));
            return new HeuristicCallback(env, this,
                                         new routing::Generator(new CVRPTW::Constructor, new CVRPTW::RandomDestructor),
                                         neighborhoods);

        }
    }

    neighborhoods.push_back(new CVRPTW::Shift());
    return new HeuristicCallback(env, this,
                                 new routing::Generator(new CVRPTW::Constructor, new CVRPTW::RandomDestructor),
                                 neighborhoods);

}


void CVRPTW::HeuristicCallback::main()
{
    initSolution();
    routing::callback::HeuristicCallback::main();
    delete solution;
}

void CVRPTW::HeuristicCallback::extractSolution() {
    unsigned t = 0, last = 0;
    Tour * tour = new Tour(problem, t, static_cast<CVRPTW::Solution*>(solution)->visits);

    do {
        unsigned foundtime = 0;


        for (unsigned j = 0; j < problem->arcs.size(); ++j) {
            if (getIncumbentValue(problem->arcs[last][j]) >= 1 - 1e-6) {
                if (last == 0 && foundtime < t) {
                    foundtime++;
                    continue;
                }
                if (j != 0) {
                    solution->notserved.erase(std::remove(solution->notserved.begin(), solution->notserved.end(), static_cast<CVRPTW::Client *>(problem->clients[j - 1])),
                                              solution->notserved.end());
                    tour->pushClient(static_cast<CVRPTW::Client *>(problem->clients[j - 1]));

                }
                last = j;

                break;
            }
        }
        if (last == 0) {
            t++;
            solution->pushTour(tour);
            tour = new Tour(problem, t, static_cast<CVRPTW::Solution*>(solution)->visits);
        }

    }
    while (t < problem->vehicles.size() && solution->notserved.size() > 0);
    
    solution->pushTour(tour);
    if(t == problem->vehicles.size()) delete tour;
    while(solution->getNbTour() < problem->vehicles.size()) {
        solution->pushTour(new Tour(problem, solution->getNbTour(),static_cast<CVRPTW::Solution*>(solution)->visits));
    }
}


routing::models::Solution* CVRPTW::HeuristicCallback::extractPartialSolution(routing::Problem* problem)
{
    CVRPTW::Solution* solution = new CVRPTW::Solution(static_cast<CVRPTW::Problem*>(problem));

    solution->toRoute.clear();
    unsigned t = 0, last = 0;


    std::vector<int> arcSet;
    for(auto k = 0; k < problem->arcs.size(); ++k){
        arcSet.push_back(k);
    }

    Tour * tour = new Tour(static_cast<CVRPTW::Problem*>(problem), t, solution->visits);
    unsigned int s_ = 1;
    do {
        unsigned foundtime = 0;
        s_ = 1;
        unsigned k = 0;
        while (k < arcSet.size()) {
            unsigned j = arcSet[k];
            if (std::abs(getValue(problem->arcs[last][j])  - 1 ) <= Configuration::epsilon ) {
                if (last == 0 && foundtime < t) {
                    foundtime++;
                    continue;
                }
                if (j != 0) {
                    solution->notserved.erase(std::remove(solution->notserved.begin(), solution->notserved.end(), static_cast<CVRPTW::Client *>(problem->clients[j - 1])),
                                              solution->notserved.end());
                    tour->pushClient(static_cast<CVRPTW::Client *>(problem->clients[j - 1]));
                    arcSet.erase(std::remove(arcSet.begin(),arcSet.end(),j));
                } else{
                    k++;
                }

                last = j;
                s_ = 1;
                break;
            }else{
                if(std::abs(getValue(problem->arcs[last][j])) > Configuration::epsilon ){
                    if (j != 0 ){
                        arcSet.erase(std::remove(arcSet.begin(),arcSet.end(),j));
                        solution->toRoute.push_back(static_cast<CVRPTW::Client *>(problem->clients[j - 1]));
                    }
                    last = 0;
                    s_ = 1;
                    k = 0;
                    break;
                }else {
                    s_ ++;
                    k++;
                }

            }


        }//end while
        if (last == 0) {
            t++;
            solution->pushTour(tour);
            tour = new Tour(static_cast<CVRPTW::Problem*>(problem), t, solution->visits);
        }

    }
    while (t < problem->vehicles.size() && solution->notserved.size() > 0 && s_ < arcSet.size());

    solution->pushTour(tour);
    if(t == problem->vehicles.size()) delete tour;

    //build empty tours for each unused vechicle
    while(solution->getNbTour() < problem->vehicles.size()) {
        solution->pushTour(new Tour(static_cast<CVRPTW::Problem*>(problem), solution->getNbTour(),solution->visits));
    }

    return solution;
}


/***
 *
 * @param problem
 * @return forbiddenPositions
 *
 * this function tries to find for each position i, the position j that is set to one
 * thus no insertion would be made possible between i and j
 */
routing::forbiddenPositions CVRPTW::HeuristicCallback::extractForbiddenPositions(routing::Problem* problem) {
    routing::forbiddenPositions fp;

    for(auto i = 0 ; i < problem->clients.size(); ++i){
        fp.insert(std::make_pair(problem->clients[i]->getID(),std::vector<unsigned int>()));
    }


    for(unsigned i = 1 ; i < problem->arcs.size(); ++i){
        //if i is not the last client to route
        if(std::abs(getValue(problem->arcs[i][0]) - 1) > Configuration::epsilon)
        {
            for (unsigned j = 1; j < problem->arcs.size(); ++j) {
                if (i == j ) continue;
                //if there can't be an arc between i and j then j is in the forbidden positions of i
                if (std::abs(getValue(problem->arcs[i][j])) <= Configuration::epsilon)
                {
                    fp[problem->clients[i-1]->getID()].push_back(problem->clients[j-1]->getID());
                }

            }
        }
    }
    return fp;
}

