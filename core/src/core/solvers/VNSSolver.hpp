//
// Created by Sohaib LAFIFI on 22/11/2019.
//

#pragma once

#include "Solver.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/neighborhoods/Neighborhood.hpp"
#include <cassert>

namespace routing {
    template<class Reader>
    class VNSSolver : public Solver<Reader> {
    protected:
        std::vector<routing::Neighborhood *> neighbors;
        routing::Generator *generator = nullptr;
    public:
        VNSSolver(const std::string &p_inputFile,
                  Generator *p_generator,
                  const std::vector<Neighborhood *> &p_neighbors,
                  std::ostream &os = std::cout) : Solver<Reader>(p_inputFile, os),
                                                  generator(p_generator),
                                                  neighbors(p_neighbors) {
        this->setDefaultConfiguration();

        }

        VNSSolver(const std::string &p_inputFile,
                  std::ostream &os = std::cout) : Solver<Reader>(p_inputFile, os) {
        this->setDefaultConfiguration();

        }

        virtual void setGenerator(Generator *p_generator) { this->generator = p_generator; }

        virtual void setNeighbors(std::vector<routing::Neighborhood *> p_neighbors) { this->neighbors = p_neighbors; }

        virtual void shake(models::Solution *solution) {
            routing::models::Solution *save = solution->clone();
            generator->getDestructor()->destruct(solution);
            bool solutionFound = generator->getConstructor()->bestInsertion(solution);
            if (!solutionFound) solution->copy(save);
        }

        virtual void setDefaultConfiguration() override {
            this->configuration = new Configuration();
            this->configuration->setIntParam(this->configuration->itermax,
                    this->problem->clients.size() * this->problem->clients.size());
        };

        virtual bool solve(double timeout = 3600) override {
            assert(generator != nullptr);
            this->solution = generator->generate();
            std::random_device rd;
            int itermax = this->configuration->getIntParam(this->configuration->itermax);
            int iter = 1;
            routing::models::Solution *best = this->solution->clone();
            double bestCost = this->solution->getCost();
            while (iter++ < itermax) {
                shake(this->solution);
                std::vector<bool> run(neighbors.size(), false);
                while (std::find(run.begin(), run.end(), false) != run.end()) {
                    unsigned i = 0;
                    do { i = rd() % run.size(); } while (run[i]);
                    if (neighbors[i]->look(this->solution)) {
                        run = std::vector<bool>(neighbors.size(), false);
                    } else {
                        run[i] = true;
                    }
                    if (this->solution->getCost() < bestCost - 1e-9) {
                        bestCost = this->solution->getCost();
                        std::cout << bestCost << std::endl;
                        iter = 1;
                        best->copy(this->solution);
                    }
                }
            }
            this->solution->copy(best);
            this->os << this->problem->getName()
                     << "\t" << this->solution->getCost()
                     << std::endl;
            this->solution->print(this->os);
            return this->solution != nullptr;
        }

        virtual ~VNSSolver() = default;
    };
}
