//
// Created by Sohaib LAFIFI on 29/11/2019.
//

#include "Problem.hpp"

routing::Initializer *cvrptw::Problem::initializer() {
    return new cvrptw::Initializer(this);
}
