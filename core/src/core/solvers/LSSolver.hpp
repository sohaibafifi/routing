//
// Created by Sohaib LAFIFI on 22/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "Solver.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/neighborhoods/Neighborhood.hpp"
#include <cassert>

namespace routing {
    template<class Reader>
    class LSSolver : public Solver<Reader> {
    protected:
        std::vector<routing::Neighborhood *> neighbors;
        routing::Generator *generator = nullptr;
    public:
        LSSolver(const std::string &p_inputFile,
                 Generator *p_generator,
                 const std::vector<Neighborhood *> &p_neighbors,
                 std::ostream &os = std::cout) : Solver<Reader>(p_inputFile, os),
                                                 generator(p_generator),
                                                 neighbors(p_neighbors) {
            this->setDefaultConfiguration();

        }

        LSSolver(const std::string &p_inputFile,
                 std::ostream &os = std::cout) : Solver<Reader>(p_inputFile, os) {
            this->setDefaultConfiguration();
        }

        virtual void setGenerator(Generator *p_generator) { this->generator = p_generator; }

        virtual void setNeighbors(std::vector<routing::Neighborhood *> p_neighbors) { this->neighbors = p_neighbors; }

        virtual void setDefaultConfiguration() override {
            this->configuration = new Configuration();
        };

        virtual bool solve(double timeout = 3600) override {
            assert(generator != nullptr);
            this->solution = generator->generate();
            std::random_device rd;

            std::vector<bool> run(neighbors.size(), false);
            while (std::find(run.begin(), run.end(), false) != run.end()) {
                unsigned i = 0;
                do { i = rd() % run.size(); } while (run[i]);
                if (neighbors[i]->look(this->solution)) {
                    run = std::vector<bool>(neighbors.size(), false);
                } else {
                    run[i] = true;
                }
            }
            this->os << this->problem->getName()
                     << "\t" << this->solution->getCost()
                     << std::endl;
            this->solution->print(this->os);
            return this->solution != nullptr;
        }

        virtual ~LSSolver() = default;
    };
}
