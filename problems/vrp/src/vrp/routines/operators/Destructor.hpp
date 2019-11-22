//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <core/routines/operators/Destructor.hpp>
#include "../../models/Solution.hpp"

namespace vrp {
    namespace routines {
        struct RandomDestructionParameters : routing::DestructionParameters {
            unsigned long getDmax() {
                return this->dmax;
            }

            unsigned long dmax;

            RandomDestructionParameters(unsigned long dmax) : dmax(dmax) {}

            RandomDestructionParameters(routing::Problem *problem) : dmax(problem->vehicles.size()) {}

            void setDmax(unsigned long dmax) {
                RandomDestructionParameters::dmax = dmax;
            }
        };

        class Destructor : public routing::Destructor {

        public:
            Destructor(routing::DestructionParameters *p_params) : routing::Destructor(p_params) {}

        private:
            virtual void destruct(routing::models::Solution *solution) {
                RandomDestructionParameters *parameters = static_cast<RandomDestructionParameters *>(params);
                if (solution->notserved.size() == solution->getProblem()->clients.size()) return;
                std::random_device rd;

                unsigned drem = 1 + rd() % parameters->getDmax();
                do {
                    unsigned t = rd() % solution->getNbTour();
                    while (static_cast<models::Tour *>(solution->getTour(t))->getNbClient() < 1) {
                        t = rd() % solution->getNbTour();
                    }
                    unsigned long position = (rd() % static_cast<models::Tour *>(solution->getTour(t))->getNbClient());
                    models::Client *client = static_cast<models::Client *>(solution->getTour(t)->getClient(position));
                    static_cast<models::Solution *>(solution)->notserved.push_back(client);
                    routing::Duration traveltime = static_cast<models::Tour *>(solution->getTour(t))->getTraveltime();
                    solution->getTour(t)->removeClient(position);
                    static_cast<models::Solution *>(solution)->traveltime =
                            static_cast<models::Solution *>(solution)->traveltime
                            - traveltime
                            + static_cast<models::Tour *>(solution->getTour(t))->getTraveltime();

                } while (static_cast<models::Solution *>(solution)->notserved.size() < drem);
            }

        };
    }
}