//
// Created by Sohaib LAFIFI on 21/11/2019.
//

#pragma once

#include <core/data/models/Tour.hpp>
#include <core/data/attributes/Consumer.hpp>
#include <core/data/attributes/Stock.hpp>
#include "Client.hpp"

namespace vrp {
    namespace models {

        class Tour : public routing::models::Tour {
        protected:
            std::vector<Client *> clients;
            routing::Duration traveltime;
            routing::Demand consumption;
        public:

            Tour(routing::Problem *p_problem, unsigned vehicleID) :
                    routing::models::Tour(p_problem, vehicleID),
                    traveltime(0),
                    consumption(0),
                    clients(std::vector<Client *>()) {}

            Tour *clone() const override {
                Tour *tour = new Tour(this->problem, this->getID());
                *tour = *this;
                return tour;
            }

            routing::Duration getTraveltime() const {
                return traveltime;
            }

            routing::Duration getConsumption() const {
                return consumption;
            }

            void pushClient(routing::models::Client *client) override {
                traveltime += static_cast<routing::InsertionCost *>(Tour::evaluateInsertion(client,
                                                                                            getNbClient()))->getDelta();
                if (routing::attributes::Consumer *consumer = dynamic_cast<routing::attributes::Consumer *>(client))
                    consumption += consumer->getDemand();
                clients.insert(clients.begin() + getNbClient(), static_cast<Client *>(client));
            }

            void addClient(routing::models::Client *client, unsigned long position) override {
                //TODO : optimize to not recalculate the insertion cost
                traveltime += this->evaluateInsertion(client, position)->getDelta();
                if (routing::attributes::Consumer *consumer = dynamic_cast<routing::attributes::Consumer *>(client))
                    consumption += consumer->getDemand();
                clients.insert(clients.begin() + position, static_cast<Client *>(client));
            }

            void removeClient(unsigned long position) override {
                traveltime += static_cast<routing::RemoveCost *>(this->evaluateRemove(position))->getDelta();
                if (routing::attributes::Consumer *consumer = dynamic_cast<routing::attributes::Consumer *>(getClient(
                        position)))
                    consumption -= consumer->getDemand();
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
                routing::attributes::Consumer *consumer = dynamic_cast<routing::attributes::Consumer *>(client);
                routing::attributes::Stock *stock = dynamic_cast<routing::attributes::Stock *>(problem->vehicles[this->getID()]);

                if (consumer && stock && consumer->getDemand() + getConsumption() > stock->getCapacity())
                    return new routing::InsertionCost(0, false);
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
                        delta = -2 * problem->getDistance(*client, *static_cast< Problem *>(problem)->getDepot());
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

