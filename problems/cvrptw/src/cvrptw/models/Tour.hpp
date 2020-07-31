// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <cvrp/models/Tour.hpp>
#include "Client.hpp"

namespace cvrptw {
    namespace models {
        class InsertionCost : public routing::InsertionCost {
        private:
            routing::Duration start;
            routing::Duration wait;
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

            InsertionCost(routing::Duration delta, routing::Duration start, routing::Duration wait, bool possible)
                    : routing::InsertionCost(delta, possible), start(start), wait(wait) {}

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

            void update() override {}

            void pushClient(routing::models::Client *client) override {
                vrp::models::Tour::pushClient(client);
                visits.push_back(new Visit(dynamic_cast<Client *>(client), 0, 0, 0));
                this->update();
            }

            void addClient(routing::models::Client *client, unsigned long position) override {
                //TODO : optimize to not recalculate the insertion cost
                InsertionCost *insertion = static_cast<InsertionCost *>(this->evaluateInsertion(client, position));
                traveltime += insertion->getDelta();
                if (auto *consumer = dynamic_cast<routing::attributes::Consumer *>(client))
                    consumption += consumer->getDemand();
                clients.insert(clients.begin() + position, dynamic_cast<Client *>(client));

                // vrp::models::Tour::addClient(client, position);
                visits.insert(visits.begin() + position, new Visit(
                        dynamic_cast<Client *>(client),
                        insertion->getStart(),
                        0,
                        insertion->getWait()));
                this->update();
                updated = true;
            }

            void removeClient(unsigned long position) override {
                vrp::models::Tour::removeClient(position);
                visits.erase(visits.begin() + position);
            }

            void clear() override {
                vrp::models::Tour::clear();
                visits = std::vector<Visit *>();
            }

            unsigned long getNbClient() override {
                return this->visits.size();
            }


            routing::InsertionCost *
            evaluateInsertion(routing::models::Client *client, unsigned long position) override {
                return vrp::models::Tour::evaluateInsertion(client, position);
            }

            routing::RemoveCost *evaluateRemove(unsigned long position) override {
                return vrp::models::Tour::evaluateRemove(position);
            }

        };
    }
}


