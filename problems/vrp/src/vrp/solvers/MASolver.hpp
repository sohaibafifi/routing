// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <plugins/solvers/MASolverPlugin/MASolver.hpp>
#include "../models/Solution.hpp"
#include "../routines/operators/Constructor.hpp"
#include "../routines/operators/Destructor.hpp"
#include <plugins/neighborhoods/NeighborhoodCorePlugin/Neighborhood.hpp>
#include <plugins/neighborhoods/IDCHPlugin/IDCH.hpp>
#include <plugins/neighborhoods/NeighborhoodCorePlugin/Move.hpp>
#include <plugins/neighborhoods/TwoOptPlugin/TwoOpt.hpp>

namespace vrp {
    template<class Reader>
    class MASolver : public routing::MASolver<Reader> {


    public:
        explicit MASolver(const std::string &p_inputFile,
                          std::ostream &os = std::cout) :
                routing::MASolver<Reader>(p_inputFile, os) {
            this->setGenerator(new routing::Generator(
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

            neighbors.push_back(new routing::TwoOpt());
            this->setNeighbors(neighbors);

        }
    };
}