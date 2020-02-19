//
// Created by Sohaib LAFIFI on 22/11/2019.
//

#pragma once

#include <core/solvers/GASolver.hpp>
#include "../models/Solution.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/operators/Constructor.hpp"
#include "../routines/operators/Destructor.hpp"
#include <core/routines/neighborhoods/Neighborhood.hpp>
#include <core/routines/neighborhoods/IDCH.hpp>
#include <core/routines/neighborhoods/Move.hpp>

namespace vrp {
    template<class Reader>
    class GASolver : public routing::GASolver<Reader> {


    public:
        explicit GASolver(const std::string &p_inputFile,

                 std::ostream &os = std::cout) :
                routing::GASolver<Reader>(p_inputFile, os) {
            this->setGenerator(new routines::Generator(
                    this->problem,
                    new vrp::routines::Constructor(),
                    new vrp::routines::Destructor()
            ));

            std::vector<routing::Neighborhood *> neighbors = std::vector<routing::Neighborhood *>();
            this->setNeighbors(neighbors);

        }
    };
}