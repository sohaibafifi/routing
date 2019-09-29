#pragma once
#include <sstream>
#include "../../../data/models/Tour.hpp"
#include "../Problem.hpp"
#include "Client.hpp"
#include "InsertionCost.hpp"
#include "RemoveCost.hpp"

namespace CVRP
{
    struct Tour : public routing::models::Tour{

        public:
            Tour(Problem * p_problem, unsigned vehicleID):
                routing::models::Tour(p_problem, vehicleID),
                traveltime(0), consumption(0),
                clients(std::vector<Client*>()){}
            Tour(){}
            virtual void pushClient(routing::models::Client *client) override;
            virtual void addClient(routing::models::Client *client, unsigned long position) override;
            virtual routing::InsertionCost* evaluateInsertion(routing::models::Client *client, unsigned long position) override;
            virtual routing::RemoveCost* evaluateRemove(unsigned long position) override;
            virtual void removeClient(unsigned long position) override;
            virtual routing::models::Client *getClient(unsigned long c) const override;
            virtual unsigned long getNbClient() override;
            routing::Duration traveltime;
            routing::Demand consumption;
        protected:
            std::vector<Client*> clients;

    };

}
