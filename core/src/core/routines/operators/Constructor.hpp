#pragma once

#include "../../data/models/Solution.hpp"

namespace routing {

    class Constructor {
    public:
        virtual bool bestInsertion(models::Solution *solution){
            return this->bestInsertion(solution, solution->notserved);
        }
        virtual bool bestInsertion(models::Solution *solution, models::Client *client) {
            std::vector<models::Client*> clients = std::vector<models::Client*>();
            clients.push_back(client);
            return this->bestInsertion(solution, clients);
        }
        virtual bool bestInsertion(models::Solution *solution, const std::vector<models::Client*> clients) = 0;
    };

    class dummyConstructor : public Constructor {
    public:
        virtual bool bestInsertion(models::Solution *solution, std::vector<models::Client*> clients)  { return false; }
    };
}
