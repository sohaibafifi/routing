//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <core/data/models/Tour.hpp>

namespace vrp {
    namespace models {

        class Tour : public routing::models::Tour {
        protected:
            std::vector<Client *> clients;
            routing::Duration traveltime;
        public:
            routing::Duration getTraveltime() const {
                return traveltime;
            }

            void pushClient(routing::models::Client *client) override {
                traveltime += static_cast<routing::InsertionCost *>(Tour::evaluateInsertion(client,
                                                                                            getNbClient()))->getDelta();
                clients.insert(clients.begin() + getNbClient(), static_cast<Client *>(client));
            }

            void addClient(routing::models::Client *client, unsigned long position) override {
                bool possible = true; // we assume that this is already verified
                //TODO : optimize to not recalculate the insertion cost
                traveltime += static_cast<routing::InsertionCost *>(this->evaluateInsertion(client,
                                                                                            position))->getDelta();
                clients.insert(clients.begin() + position, static_cast<Client *>(client));
            }

            void removeClient(unsigned long position) override {
                traveltime += static_cast<routing::RemoveCost *>(this->evaluateRemove(position))->getDelta();
                clients.erase(clients.begin() + position);
            }

            routing::models::Client *getClient(unsigned long i) const override {
                return clients[i];
            }

            unsigned long getNbClient() override {
                return clients.size();
            }

            routing::InsertionCost *
            evaluateInsertion(routing::models::Client *client, unsigned long position) override {
                routing::InsertionCost *cost = new routing::InsertionCost();


                routing::Duration delta = 0;
                if (clients.empty()) {
                    delta = 2 * problem->getDistance(*client, *static_cast<Problem *>(problem)->getDepot());
                } else {
                    if (position == 0) {
                        delta = problem->getDistance(*client, *clients[position])
                                + problem->getDistance(*client, *static_cast<Problem *>(problem)->getDepot())
                                -
                                problem->getDistance(*clients[position], *static_cast<Problem *>(problem)->getDepot());
                    } else {
                        if (position == clients.size()) {
                            delta = problem->getDistance(*clients[position - 1], *client)
                                    + problem->getDistance(*client, *static_cast<Problem *>(problem)->getDepot())
                                    - problem->getDistance(*clients[position - 1],
                                                           *static_cast<Problem *>(problem)->getDepot());

                        } else {
                            delta = problem->getDistance(*clients[position - 1], *client)
                                    + problem->getDistance(*client, *clients[position])
                                    - problem->getDistance(*clients[position - 1], *clients[position]);
                        }
                    }
                }
                cost->setPossible(true);
                cost->setDelta(delta);
                return cost;
            }

            routing::RemoveCost *evaluateRemove(unsigned long position) override {
                Client *client = clients[position];
                routing::Duration delta = 0;
                if (position == 0) {
                    if (position == clients.size() - 1) {
                        delta = 2 * problem->getDistance(*client, *static_cast< Problem *>(problem)->getDepot());
                    } else {
                        delta = -problem->getDistance(*client, *static_cast< Problem *>(problem)->getDepot())
                                - problem->getDistance(*client, *clients[position + 1])
                                + problem->getDistance(*clients[position + 1],
                                                       *static_cast< Problem *>(problem)->getDepot());
                    }
                } else {
                    if (position == clients.size() - 1) {
                        delta = -problem->getDistance(*clients[position - 1], *client)
                                - problem->getDistance(*client, *static_cast< Problem *>(problem)->getDepot())
                                + problem->getDistance(*clients[position - 1],
                                                       *static_cast< Problem *>(problem)->getDepot());

                    } else {
                        delta = -problem->getDistance(*clients[position - 1], *client)
                                - problem->getDistance(*client, *clients[position + 1])
                                + problem->getDistance(*clients[position - 1], *clients[position + 1]);
                    }
                }
                return new routing::RemoveCost(delta);
            }


        };
    }
}

