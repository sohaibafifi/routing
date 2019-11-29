//
// Created by Sohaib LAFIFI on 29/11/2019.
//

#pragma once

#include <vrp/models/Tour.hpp>
#include "Client.hpp"

namespace cvrptw {
    namespace models {
        class Tour : public vrp::models::Tour {
        protected:
            routing::Duration  totalTime;
            Tour *clone() const override {
                Tour *tour = new Tour(this->problem, this->getID());
                *tour = *this;
                return tour;
            }

        public:
            models::Client *getClient(unsigned long i) const override {
                 return dynamic_cast<Client *>(clients[i]);
            }

        public:
            Tour(routing::Problem *p_problem, unsigned vehicleID) :
                    vrp::models::Tour(p_problem, vehicleID),
                    totalTime(0){}
        };
    }
}


