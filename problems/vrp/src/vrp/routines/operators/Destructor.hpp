//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <core/routines/operators/Destructor.hpp>
#include "../../models/Solution.hpp"
#include <cassert>

namespace vrp {
    namespace routines {
        struct RandomDestructionParameters : routing::DestructionParameters {
            unsigned long getDmax() {
                return this->dmax;
            }

            unsigned long dmax;

            explicit RandomDestructionParameters(unsigned long dmax) : dmax(dmax) {}

            explicit RandomDestructionParameters(routing::Problem *problem) : dmax(
                    std::min(problem->vehicles.size(), problem->clients.size())) {}

            void setDmax(unsigned long dmax) {
                RandomDestructionParameters::dmax = dmax;
            }
        };

        class Destructor : public routing::Destructor {

        public:
            explicit Destructor(routing::DestructionParameters *p_params) : routing::Destructor(p_params) {}

        private:
             void destruct(routing::models::Solution *solution) override {
                // assert(solution->notserved.empty());
                assert(solution->getNbTour() > 0);
                RandomDestructionParameters *parameters = static_cast<RandomDestructionParameters *>(params);
                if (solution->notserved.size() == solution->getProblem()->clients.size()) return;
                std::random_device rd;

                unsigned long drem = 1 + rd() % parameters->getDmax();
                do {
                    unsigned long t = rd() % solution->getNbTour();
                    while (dynamic_cast<models::Tour *>(solution->getTour(t))->getNbClient() < 1) {
                        t = rd() % solution->getNbTour();
                    }
                    unsigned long position = (rd() % static_cast<models::Tour *>(solution->getTour(t))->getNbClient());
                    models::Client *client = dynamic_cast<models::Client *>(solution->getTour(t)->getClient(position));
                    solution->removeClient(t, position);
                } while (dynamic_cast<models::Solution *>(solution)->notserved.size() < drem);
            }

        };
    }
}