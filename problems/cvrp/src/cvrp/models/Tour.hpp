// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <vrp/models/Tour.hpp>
#include <vrp/Problem.hpp>
#include "Client.hpp"

namespace cvrp {
    namespace models {
        class InsertionCost : public routing::InsertionCost {
        private:
            routing::Demand consumption;

        public:

            InsertionCost(routing::Duration delta, routing::Demand consumption, bool possible)
                    : routing::InsertionCost(delta, possible),
                      consumption(consumption) {

            }

            routing::Duration getConsumption() const {
                return consumption;
            }

            InsertionCost() : routing::InsertionCost(), consumption(0) {}

            InsertionCost(const InsertionCost &cost)
                    : routing::InsertionCost(cost), consumption(cost.getConsumption()) {}

        };

        class Tour : public vrp::models::Tour {
        protected:
            routing::Duration totalTime;
            routing::Demand consumption;


        public:

            Tour(routing::Problem *p_problem, unsigned vehicleID) :
                    vrp::models::Tour(p_problem, vehicleID),
                    consumption(0),
                    totalTime(0) {}

            void update() override {}

            routing::Duration getConsumption() const {
                return consumption;
            }


            void pushClient(routing::models::Client *client, routing::InsertionCost * cost) override {
                traveltime += cost->getDelta();
                updated = true;
                if (auto *consumer = dynamic_cast<routing::attributes::Consumer *>(client))
                    consumption += consumer->getDemand();
                clients.push_back(dynamic_cast<Client *>(client));
            }

            void addClient(routing::models::Client *client, unsigned long position, routing::InsertionCost * cost) override {
                updated = true;
                traveltime += cost->getDelta();
                if (auto *consumer = dynamic_cast<routing::attributes::Consumer *>(client))
                    consumption += consumer->getDemand();
                clients.insert(clients.begin() + position, dynamic_cast<Client *>(client));
            }

            void removeClient(unsigned long position) override {
                updated = true;
                traveltime += this->evaluateRemove(position)->getDelta();
                if (auto *consumer = dynamic_cast<routing::attributes::Consumer *>(getClient(
                        position)))
                    consumption -= consumer->getDemand();
                clients.erase(clients.begin() + position);
            }

            void clear() override {
                vrp::models::Tour::clear();
                consumption = 0;
            }

            routing::InsertionCost *
            evaluateInsertion(routing::models::Client *client, unsigned long position) override {
                auto *consumer = dynamic_cast<routing::attributes::Consumer *>(client);
                auto *stock = dynamic_cast<routing::attributes::Stock *>(problem->vehicles[this->getID()]);

                if (consumer && stock && consumer->getDemand() + getConsumption() > stock->getCapacity())
                    return new routing::InsertionCost(0, false);
                auto *cost = new routing::InsertionCost();

                routing::Duration delta = 0;
                if (clients.empty()) {
                    delta = 2 * problem->getDistance(*client, *static_cast<vrp::Problem *>(problem)->getDepot());
                } else {
                    if (position == 0) {
                        delta = problem->getDistance(*client, *clients[position])
                                + problem->getDistance(*client, *static_cast<vrp::Problem *>(problem)->getDepot())
                                -
                                problem->getDistance(*clients[position], *static_cast<vrp::Problem *>(problem)->getDepot());
                    } else {
                        if (position == clients.size()) {
                            delta = problem->getDistance(*clients[position - 1], *client)
                                    + problem->getDistance(*client, *static_cast<vrp::Problem *>(problem)->getDepot())
                                    - problem->getDistance(*clients[position - 1],
                                                           *static_cast<vrp::Problem *>(problem)->getDepot());

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
                return vrp::models::Tour::evaluateRemove(position);
            }

        };
    }
}


