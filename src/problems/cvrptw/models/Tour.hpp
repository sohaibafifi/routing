//
// Created by ali on 3/28/19.
//

#ifndef HYBRID_TOUR_H
#define HYBRID_TOUR_H

#include <algorithm>
#include "../../cvrp/models/Tour.hpp"
#include "Problem.hpp"
#include "Visit.hpp"
#include "InsertionCost.hpp"
#include "../../../routines/divers/Diver.hpp"

namespace CVRPTW{
    struct Tour : public CVRP::Tour{

    public:
        Tour(Problem * p_problem, unsigned vehicleID, std::vector<CVRPTW::Visit*> visits):
                CVRP::Tour(p_problem, vehicleID), visits(visits){}

        Tour(const Tour& tour){
            this->setID(tour.getID());
            this->traveltime = tour.traveltime;
            this->consumption = tour.consumption;
            this->problem = tour.problem;
            this->visits = tour.visits;
            this->clients.clear();
            for (int i = 0; i < tour.clients.size(); ++i) {
                this->clients.push_back(new Client(*(static_cast<Client*>(tour.getClient(i)))));
            }
        }
        virtual routing::InsertionCost* evaluateInsertion(routing::models::Client *client, unsigned long position) override;
        virtual routing::InsertionCost* evaluateCompletion(routing::models::Client *client, unsigned long position, routing::forbiddenPositions* forbiddenPosition);
        virtual routing::Duration evaluateRemove(unsigned long position) override;
        virtual void updateTimeVariablesAfterRemove(unsigned long position);
        virtual void removeClient(unsigned long position) override;
        virtual void addClient(routing::models::Client *client, unsigned long position) override;
        virtual routing::Duration getNextMaxshift(unsigned long position); //returns maxshift of position + 1
        virtual routing::Duration getNextWait(unsigned long position); // returns wait of position + 1
        virtual routing::Duration getArrival(unsigned long position); //get arrival at position
        std::vector<CVRPTW::Visit*> visits;
    };
}

#endif //HYBRID_TOUR_H
