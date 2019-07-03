//
// Created by ali on 6/26/19.
//

#include "Constructor_TSPTWH.hpp"
#include "../../../../routines/operators/TSPTWDestructionParameters.hpp"

bool CVRPTW::Constructor_TSPTWH::bestInsertion(routing::models::Solution *solution, routing::DestructionParameters* param)
{
    routing::TSPTWDestructionParameters* parameters = static_cast<routing::TSPTWDestructionParameters*>(param);

    CVRPTW::Tour* tour = static_cast<CVRPTW::Tour*>(solution->getTour(parameters->getTourIndex()));

    unsigned int start = parameters->getStartPosition();

    unsigned int nbClientToRoute = parameters->toRoute.size();
    unsigned int totalClient = tour->getNbClient() + nbClientToRoute ;

    unsigned int end = parameters->getEndPosition() % totalClient;

    unsigned int position = 0;


    std::vector<unsigned int > permittedPositions;
    InsertionCost* insertionCost = new InsertionCost(IloInfinity,false,IloInfinity);
    InsertionCost* bestCost = new InsertionCost(IloInfinity,false,IloInfinity);


    int i = 0;
    bool insertion_found = false;


    if(start < end ){
        i = start;
        while (i <= end){
            //push positions to check for client insertion
            permittedPositions.push_back(i);

            //save best position and best client to insert
            unsigned best_p = 0, best_client_i = 0;

            insertion_found = false;

            //for each client to route
            for (int j = 0; j < parameters->getToRoute().size(); ++j) {
                //for each position
                for (int k = 0; k <  permittedPositions.size(); ++k) {
                    position = permittedPositions[k];
                    insertionCost = static_cast<InsertionCost*>(tour->evaluateInsertion(parameters->getToRouteClient(j),position));
                    if(!insertionCost->isPossible()) continue;
                    if(insertionCost->getDelta()  < bestCost->getDelta()){
                        best_p = position;
                        best_client_i = j;
                        *bestCost = *insertionCost;
                        insertion_found = true;
                    }
                }

            }

            if (insertion_found) { //update solution

                tour->addClient(parameters->getToRouteClient(best_client_i), best_p );
                static_cast<Solution*>(solution)->traveltime += bestCost->getDelta();
                parameters->toRoute.erase(parameters->toRoute.begin() + best_client_i);
                static_cast<Solution*>(solution)->updateTimeVariables(tour,best_p,bestCost->getShift());
            }
            bestCost->setShift(IloInfinity);
            bestCost->setDelta(IloInfinity);
            bestCost->setPossible(false);

            i++;
        }

    } else if (start > end ){
        for (i = 0; i <= end ; ++i) {
            //push positions to check for client insertion
            permittedPositions.push_back(i);

            //save best position and best client to insert
            unsigned best_p = 0, best_client_i = 0;

            insertion_found = false;

            //for each client to route
            for (int j = 0; j < parameters->getToRoute().size(); ++j) {
                //for each position
                for (int k = 0; k <  permittedPositions.size(); ++k) {
                    position = permittedPositions[k];
                    insertionCost = static_cast<InsertionCost*>(tour->evaluateInsertion(parameters->getToRouteClient(j),position));
                    if(!insertionCost->isPossible()) continue;
                    if(insertionCost->getDelta()  < bestCost->getDelta()){
                        best_p = position;
                        best_client_i = j;
                        *bestCost = *insertionCost;
                        insertion_found = true;
                    }
                }

            }

            if (insertion_found) { //update solution

                tour->addClient(parameters->getToRouteClient(best_client_i), best_p );
                static_cast<Solution*>(solution)->traveltime += bestCost->getDelta();
                parameters->toRoute.erase(parameters->toRoute.begin() + best_client_i);
                static_cast<Solution*>(solution)->updateTimeVariables(tour,best_p,bestCost->getShift());
            }
            bestCost->setShift(IloInfinity);
            bestCost->setDelta(IloInfinity);
            bestCost->setPossible(false);

        }

        for (i = start; i < totalClient ; ++i) {
            //push positions to check for client insertion
            permittedPositions.push_back(i);

            //save best position and best client to insert
            unsigned best_p = 0, best_client_i = 0;

            insertion_found = false;

            //for each client to route
            for (int j = 0; j < parameters->getToRoute().size(); ++j) {
                //for each position
                for (int k = 0; k <  permittedPositions.size(); ++k) {
                    position = permittedPositions[k];
                    insertionCost = static_cast<InsertionCost*>(tour->evaluateInsertion(parameters->getToRouteClient(j),position));
                    if(!insertionCost->isPossible()) continue;
                    if(insertionCost->getDelta()  < bestCost->getDelta()){
                        best_p = position;
                        best_client_i = j;
                        *bestCost = *insertionCost;
                        insertion_found = true;
                    }
                }

            }

            if (insertion_found) { //update solution

                tour->addClient(parameters->getToRouteClient(best_client_i), best_p );
                static_cast<Solution*>(solution)->traveltime += bestCost->getDelta();
                parameters->toRoute.erase(parameters->toRoute.begin() + best_client_i);
                static_cast<Solution*>(solution)->updateTimeVariables(tour,best_p,bestCost->getShift());
            }
            bestCost->setShift(IloInfinity);
            bestCost->setDelta(IloInfinity);
            bestCost->setPossible(false);

        }
    }

    return insertion_found;

}