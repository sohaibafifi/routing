//
// Created by Sohaib LAFIFI on 22/11/2019.
//

#pragma once

#include <core/solvers/MASolver.hpp>
#include "../models/Solution.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/operators/Constructor.hpp"
#include "../routines/operators/Destructor.hpp"
#include <core/routines/neighborhoods/Neighborhood.hpp>
#include <core/routines/neighborhoods/IDCH.hpp>
#include <core/routines/neighborhoods/Move.hpp>

namespace vrp {
    template<class Reader>
    class MASolver : public routing::MASolver<Reader> {


    public:
        explicit MASolver(const std::string &p_inputFile,

                 std::ostream &os = std::cout) :
                routing::MASolver<Reader>(p_inputFile, os) {
            this->setGenerator(new routines::Generator(
                    this->problem,
                    new vrp::routines::Constructor(),
                    new vrp::routines::Destructor())

            );

            std::vector<routing::Neighborhood *> neighbors = std::vector<routing::Neighborhood *>();
            neighbors.push_back(new routing::IDCH(
                    new vrp::routines::Constructor(),
                    new vrp::routines::Destructor()
            ));
            neighbors.push_back(new routing::Move(
                    new vrp::routines::Constructor()
            ));
            this->setNeighbors(neighbors);

        }
    };
}