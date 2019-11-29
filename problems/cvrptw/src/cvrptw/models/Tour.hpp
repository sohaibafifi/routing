//
// Created by Sohaib LAFIFI on 29/11/2019.
//

#pragma once

#include <vrp/models/Tour.hpp>

namespace cvrptw {
    namespace models {
        class Tour : public vrp::models::Tour {
            Tour(routing::Problem *p_problem, unsigned vehicleID) :
                    vrp::models::Tour(p_problem, vehicleID) {}

            Tour *clone() const override {
                Tour *tour = new Tour(this->problem, this->getID());
                *tour = *this;
                return tour;
            }

        };
    }
}


