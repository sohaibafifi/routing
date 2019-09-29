#pragma once
#include "../Model.hpp"
#include "Client.hpp"
#include "../problem.hpp"
#include "../attributes/InsertionCost.hpp"
#include "../attributes/RemoveCost.hpp"
namespace routing {

    namespace models {

        struct Tour : public Model{
                Tour(Problem * p_problem, unsigned vehicleID):problem(p_problem){setID(vehicleID);}
                Tour(){}
            protected:
                Problem * problem;
            public :
                virtual void pushClient(Client* client) = 0;
                virtual void addClient(Client* client, unsigned long position) = 0;
                virtual void removeClient(unsigned long position) = 0;
                virtual Client * getClient(unsigned long) const = 0;
                virtual unsigned long getNbClient() = 0;

                virtual routing::InsertionCost* evaluateInsertion(Client *client, unsigned long position) = 0;
                virtual routing::RemoveCost* evaluateRemove(unsigned long position) = 0;
        };
    }
}
