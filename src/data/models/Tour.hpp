#pragma once
#include "../Model.hpp"
#include "Client.hpp"
#include "../problem.hpp"
#include "../attributes/InsertionCost.hpp"
namespace routing {

    namespace models {

        struct Tour : public Model{
                Tour(Problem * p_problem, unsigned vehicleID):problem(p_problem){setID(vehicleID);}
            protected:
                Problem * problem;
            public :
                virtual void pushClient(Client* client) = 0;
                virtual void addClient(Client* client, unsigned long position) = 0;
                virtual void removeClient(unsigned long position) = 0;
                virtual Client * getClient(unsigned long) = 0;
                virtual unsigned long getNbClient() = 0;

                virtual routing::InsertionCost* evaluateInsertion(Client *client, unsigned long position) = 0;
                virtual routing::Duration evaluateRemove(unsigned long position) = 0;
        };
    }
}
