// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <vrp/models/Tour.hpp>
#include "Client.hpp"

namespace cvrptw {
    namespace models {
        class Tour : public vrp::models::Tour {
        protected:
            routing::Duration totalTime;

            Tour *clone() const override {
                Tour *tour = new Tour(this->problem, this->getID());
                *tour = *this;
                return tour;
            }

        public:
            models::Client *getClient(unsigned long i) const override {
                return dynamic_cast<Client *>(clients[i]);
            }

            Tour(routing::Problem *p_problem, unsigned vehicleID) :
                    vrp::models::Tour(p_problem, vehicleID),
                    totalTime(0) {}

            void update() override {}

        };
    }
}


