#pragma once

#include "core/data/Model.hpp"
#include "Client.hpp"
#include "core/data/Problem.hpp"
#include "core/data/attributes/InsertionCost.hpp"
#include "core/data/attributes/RemoveCost.hpp"

namespace routing {

    namespace models {

        struct Tour : public Model {
            Tour(Problem *p_problem, unsigned vehicleID) : problem(p_problem) { setID(vehicleID); }

        protected:
            Problem *problem;
        public :
            virtual void pushClient(Client *client) = 0;

            virtual void addClient(Client *client, unsigned long position) = 0;

            virtual void removeClient(unsigned long position) = 0;

            virtual Client *getClient(unsigned long) const = 0;

            virtual unsigned long getNbClient() = 0;

            virtual Tour *clone() const = 0;

            virtual routing::InsertionCost *evaluateInsertion(Client *client, unsigned long position) = 0;

            virtual routing::RemoveCost *evaluateRemove(unsigned long position) = 0;
        };
    }
}
