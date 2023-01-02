// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <core/data/models/Tour.hpp>
#include <core/data/attributes/Stock.hpp>
#include "Client.hpp"

namespace vrp {
    namespace models {

        class Tour : public routing::models::Tour {
        protected:
            std::vector<Client *> clients;
            routing::Duration travelTime;
        public:


            Tour(routing::Problem *p_problem, unsigned vehicleID) :
                    routing::models::Tour(p_problem, vehicleID),
                    travelTime(0),
                    clients(std::vector<Client *>()),
                    updated(true) {}

            Tour *clone() const override {
                Tour *tour = new Tour(this->problem, this->getID());
                *tour = *this;
                return tour;
            }

            void update() override {
                this->travelTime = 0;
                for (int p = 0; p < clients.size(); ++p) {
                    if (p == 0) {
                        travelTime = problem->getDistance(
                                *clients[p], *problem->depots[0]);
                    } else {
                        travelTime += problem->getDistance(
                                *clients[p], *clients[p - 1]);
                    }
                }
                if (!clients.empty())
                    travelTime += problem->getDistance(
                            *clients[clients.size() - 1], *problem->depots[0]);


            }

            routing::Duration getTravelTime() const {
                return travelTime;
            }


            void pushClient(routing::models::Client *client, routing::InsertionCost *cost) override {
                travelTime += cost->getDelta();
                updated = true;
                clients.push_back(dynamic_cast<Client *>(client));
            }

            routing::InsertionCost * _pushClient(routing::models::Client *client) override {
                updated = true;
                auto evaluation = Tour::evaluateInsertion(client, getNbClient());
                travelTime += evaluation->getDelta();
                clients.push_back(dynamic_cast<Client *>(client));
                return evaluation;
            }

            void addClient(routing::models::Client *client, unsigned long position, routing::InsertionCost * cost) override {
                travelTime += cost->getDelta();
                clients.insert(clients.begin() + position, dynamic_cast<Client *>(client));
                this->update();
                this->updated = true;
            }

            void removeClient(unsigned long position) override {
                updated = true;
                travelTime += this->evaluateRemove(position)->getDelta();
                clients.erase(clients.begin() + position);
            }

            void clear() override {
                updated = true;
                travelTime = 0;
                clients = std::vector<Client *>();
            }


            routing::models::Client *getClient(unsigned long i) const override {
                return clients[i];
            }

            unsigned long getNbClient() override {
                return clients.size();
            }

            routing::InsertionCost *
            evaluateInsertion(routing::models::Client *client, unsigned long position) override {
                auto *cost = new routing::InsertionCost();

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

            bool updated = true;
            long hash = 0;

            long getHash() override {
                if (updated) {
                    if (clients.empty()) {
                        updated = false;
                        hash = 0;
                        return hash;
                    }
                    std::vector<bool> bit_sequence(problem->clients.size(), false);
                    std::string sequence;
                    for (auto & client : clients) {
                        bit_sequence[client->getID()] = true;
                        sequence.append(std::to_string(client->getID()));
                        sequence.push_back('-');
                    }
                    std::hash<std::string> hash_fn_sequence;
                    updated = false;
                    hash = hash_fn_sequence(sequence);
//                    std::hash<std::vector<bool>> hash_fn;
//                    updated = false;
//                    hash = hash_fn(bit_sequence);
                }
                return hash;
            }


        };
    }
}

