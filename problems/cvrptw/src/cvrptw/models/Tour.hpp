// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <cvrp/models/Tour.hpp>
#include "Client.hpp"
#include "Depot.hpp"

namespace cvrptw {
    namespace models {
        class InsertionCost : public routing::InsertionCost {
        private:
            routing::Duration start;
            routing::Duration wait;
            routing::Duration delta_time;
        public:


            routing::Duration getStart() const {
                return start;
            }

            void setStart(routing::Duration start) {
                InsertionCost::start = start;
            }

            routing::Duration getWait() const {
                return wait;
            }

            void setWait(routing::Duration wait) {
                InsertionCost::wait = wait;
            }

            routing::Duration getDeltaTime() const {
                return delta_time;
            }

            void setDeltaTime(routing::Duration deltaTime) {
                InsertionCost::delta_time = deltaTime;
            }

            InsertionCost(routing::Duration delta, routing::Duration start, routing::Duration wait,
                          routing::Duration delta_time, bool possible)
                    : routing::InsertionCost(delta, possible), start(start), delta_time(delta_time), wait(wait) {}

            InsertionCost() : routing::InsertionCost(), start(0), wait(0) {}

            InsertionCost(const InsertionCost &cost)
                    : routing::InsertionCost(cost), start(cost.getStart()), wait(cost.getWait()) {}

        };

        class Tour : public cvrp::models::Tour {
        protected:
            routing::Duration totalTime;
            std::vector<Visit *> visits;

            Tour *clone() const override {
                Tour *tour = new Tour(this->problem, this->getID());
                *tour = *this;
                return tour;
            }

        public:
            models::Client *getClient(unsigned long i) const override {
                return dynamic_cast<Client *>(visits[i]->client);
            }

            Tour(routing::Problem *p_problem, unsigned vehicleID) :
                    cvrp::models::Tour(p_problem, vehicleID),
                    visits(std::vector<Visit *>()),
                    totalTime(0) {}

            void update() override {
                cvrp::models::Tour::update();
                double arrival = 0;
                this->travelTime = 0;
                for (int p = 0; p < visits.size(); ++p) {
                    Visit *visit = visits.at(p);
                    if (p == 0) {
                        arrival = problem->getDistance(
                                *(visit->client), *problem->depots[0]);
                        travelTime = problem->getDistance(
                                *(visit->client), *problem->depots[0]);
                    } else {
                        arrival = visits.at(p - 1)->getStart()
                                  + visits.at(p - 1)->client->getService()
                                  + problem->getDistance(
                                *(visit->client), *visits.at(p - 1)->client);
                        travelTime += problem->getDistance(
                                *(visit->client), *visits.at(p - 1)->client);
                    }
                    visit->setStart(std::max(visit->client->getEST(), arrival));
                    visit->setWait(visit->getStart() - arrival);
                }
                if (!visits.empty())
                    travelTime += problem->getDistance(
                            *(visits.at(visits.size() - 1)->client), *problem->depots[0]);

                double waitS = 0;
                double maxShiftS = std::numeric_limits<double>::infinity();
                for (int v = visits.size() - 1; v >= 0; v--) {
                    Visit *visit = visits.at(v);
                    double maxShift = std::min(visit->client->getLST() - visit->getStart(), waitS + maxShiftS);
                    visit->setMaxshift(maxShift);
                    waitS = visit->getWait();
                    maxShiftS = maxShift;
                }
                if (!visits.empty()) // can be done using insertionCost deltaTime
                    totalTime = visits.at(visits.size() - 1)->getStart()
                                + visits.at(visits.size() - 1)->client->getService()
                                + problem->getDistance(
                            *(visits.at(visits.size() - 1)->client), *problem->depots[0]);

            }

            void pushClient(routing::models::Client *client, routing::InsertionCost *cost) override {
                vrp::models::Tour::pushClient(client, cost);
                visits.push_back(new Visit(dynamic_cast<Client *>(client), 0, 0, 0));
                updated = false; // unfinished work
                this->update();
                updated = true;
            }

            void
            addClient(routing::models::Client *client, unsigned long position, routing::InsertionCost *cost) override {
                InsertionCost *insertion = static_cast<InsertionCost *>(cost);
                travelTime += insertion->getDelta();
                if (auto *consumer = dynamic_cast<routing::attributes::Consumer *>(client))
                    consumption += consumer->getDemand();
                clients.insert(clients.begin() + position, dynamic_cast<Client *>(client));

                visits.insert(visits.begin() + position, new Visit(
                        dynamic_cast<Client *>(client),
                        insertion->getStart(),
                        0,
                        insertion->getWait()));
                updated = false; // unfinished work
                this->update();
                updated = true;
            }

            void removeClient(unsigned long position) override {
                vrp::models::Tour::removeClient(position);
                visits.erase(visits.begin() + position);
                updated = false; // unfinished work
                this->update();
                updated = true;
            }

            void clear() override {
                vrp::models::Tour::clear();
                visits = std::vector<Visit *>();
            }

            unsigned long getNbClient() override {
                return this->visits.size();
            }

            Visit *getVisit(unsigned position) {
                return this->visits.at(position);
            }


            routing::InsertionCost *
            evaluateInsertion(routing::models::Client *client, unsigned long position) override {
                auto *consumer = dynamic_cast<routing::attributes::Consumer *>(client);
                auto *stock = dynamic_cast<routing::attributes::Stock *>(problem->vehicles[this->getID()]);

                if (consumer && stock && consumer->getDemand() + getConsumption() > stock->getCapacity())
                    return new routing::InsertionCost(0, false);
                auto *cost = new InsertionCost();
                // insertion of c between i and j
                routing::Duration delta = 0, delta_time = 0;
                routing::Duration tic = 0, tij = 0, tcj = 0, arrival = 0;

                if (position == 0) {
                    tic = problem->getDistance(*client, *static_cast<vrp::Problem *>(problem)->getDepot());
                    arrival = tic;
                } else {
                    tic = problem->getDistance(*clients[position - 1], *client);
                    arrival = visits.at(position - 1)->getStart()
                              + visits.at(position - 1)->client->getService() + tic;
                }
                if (arrival > dynamic_cast<Client *>(client)->getLST())
                    return new routing::InsertionCost(0, false);

                if (position == visits.size())
                    tcj = problem->getDistance(*client, *static_cast<vrp::Problem *>(problem)->getDepot());
                else
                    tcj = problem->getDistance(*client, *clients[position]);

                if (visits.empty())
                    tij = 0.0;
                else if (position == 0)
                    tij = problem->getDistance(*client, *static_cast<vrp::Problem *>(problem)->getDepot());
                else if (position == visits.size())
                    tij = problem->getDistance(*clients[position - 1],
                                               *static_cast<vrp::Problem *>(problem)->getDepot());
                else
                    tij = problem->getDistance(*clients[position - 1], *clients[position]);
                delta_time = tic
                             + std::max(0.0, dynamic_cast<Client *>(client)->getEST() - arrival)
                             + dynamic_cast<Client *>(client)->getService()
                             + tcj
                             - tij;
                delta = tic + tcj - tij;
                routing::Duration wait_j = 0, maxShift_j = 0;
                if (position == visits.size()) {
                    wait_j = 0;
                    maxShift_j = dynamic_cast<Depot *>(problem->depots[0])->getTwClose() - totalTime;
                } else {
                    wait_j = visits[position]->getWait();
                    maxShift_j = visits[position]->getMaxshift();
                }
                cost->setDelta(delta);
                cost->setDeltaTime(delta_time);
                cost->setStart(std::max(dynamic_cast<Client *>(client)->getEST(), arrival));
                cost->setWait(cost->getStart() - arrival);
                cost->setPossible(delta_time <= wait_j + maxShift_j);
                return cost;
            }

            routing::RemoveCost *evaluateRemove(unsigned long position) override {
                return vrp::models::Tour::evaluateRemove(position);
            }

        };
    }
}


