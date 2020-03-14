// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../../data/models/Solution.hpp"

namespace routing {
    class ConstructionParameters {
    public :
        static ConstructionParameters *getDefault() {
            return new ConstructionParameters;
        }
    };

    class Constructor {
    public:
        routing::ConstructionParameters *params = nullptr;

        Constructor(routing::ConstructionParameters *p_params) : params(p_params) {}

        Constructor() {}

        Constructor *setParams(routing::ConstructionParameters *p_params) {
            this->params = p_params;
            return this;
        }

        virtual bool bestInsertion(models::Solution *solution) {
            if (this->params == nullptr) this->params = ConstructionParameters::getDefault();
            return this->bestInsertion(solution, solution->notserved);
        }

        virtual bool insertClient(models::Solution *solution, models::Client *client) {
            std::vector<models::Client *> clients = std::vector<models::Client *>();
            clients.push_back(client);
            return this->bestInsertion(solution, clients);
        }

        virtual bool bestInsertion(models::Solution *solution, const std::vector<models::Client *> clients) = 0;
    };
}
