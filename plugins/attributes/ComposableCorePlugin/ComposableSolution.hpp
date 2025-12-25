// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/Solution.hpp"
#include "models/Tour.hpp"
#include "ComposableEntity.hpp"
#include <vector>
#include <iostream>

namespace routing {

    class ComposableProblem;

    /**
     * @brief Minimal Tour implementation for ComposableProblem
     */
    class ComposableTour : public models::Tour {
    public:
        ComposableTour(Problem* p_problem, unsigned vehicleID)
            : models::Tour(p_problem, vehicleID), cost_(0) {}

        void update() override {
            // Recalculate cost based on route
            cost_ = 0;
            // Simple distance calculation would go here
        }

        void pushClient(models::Client* client, InsertionCost* cost) override {
            clients_.push_back(client);
            if (cost) cost_ += cost->getDelta();
        }

        InsertionCost* _pushClient(models::Client* client) override {
            clients_.push_back(client);
            return new InsertionCost(0, true);  // Minimal: always feasible
        }

        void addClient(models::Client* client, unsigned long position, InsertionCost* cost) override {
            if (position >= clients_.size()) {
                clients_.push_back(client);
            } else {
                clients_.insert(clients_.begin() + position, client);
            }
            if (cost) cost_ += cost->getDelta();
        }

        void removeClient(unsigned long position) override {
            if (position < clients_.size()) {
                clients_.erase(clients_.begin() + position);
            }
        }

        void clear() override {
            clients_.clear();
            cost_ = 0;
        }

        models::Client* getClient(unsigned long pos) const override {
            return pos < clients_.size() ? clients_[pos] : nullptr;
        }

        unsigned long getNbClient() override {
            return clients_.size();
        }

        models::Tour* clone() const override {
            auto* copy = new ComposableTour(problem, getID());
            copy->clients_ = clients_;
            copy->cost_ = cost_;
            return copy;
        }

        InsertionCost* evaluateInsertion(models::Client* client, unsigned long position) override {
            // Minimal: always feasible with zero cost
            return new InsertionCost(0, true);
        }

        RemoveCost* evaluateRemove(unsigned long position) override {
            return new RemoveCost(0);
        }

        long getHash() override {
            long hash = 0;
            for (auto* c : clients_) {
                hash = hash * 31 + c->getID();
            }
            return hash;
        }

        double getCost() const { return cost_; }

        const std::vector<models::Client*>& getClients() const { return clients_; }

    private:
        std::vector<models::Client*> clients_;
        double cost_;
    };

    /**
     * @brief Minimal Solution implementation for ComposableProblem
     */
    class ComposableSolution : public models::Solution {
    public:
        explicit ComposableSolution(Problem* p_problem)
            : models::Solution(p_problem), totalCost_(0) {}

        ComposableSolution(const ComposableSolution& other)
            : models::Solution(other.problem), totalCost_(other.totalCost_) {
            copy(&other);
        }

        ~ComposableSolution() override {
            for (auto* tour : tours_) {
                delete tour;
            }
        }

        void update() override {
            totalCost_ = 0;
            for (auto* tour : tours_) {
                tour->update();
                totalCost_ += dynamic_cast<ComposableTour*>(tour)->getCost();
            }
        }

        double getCost() override {
            return totalCost_;
        }

        void pushTour(models::Tour* tour) override {
            tours_.push_back(dynamic_cast<ComposableTour*>(tour));
        }

        void addTour(models::Tour* tour, unsigned long position) override {
            if (position >= tours_.size()) {
                tours_.push_back(dynamic_cast<ComposableTour*>(tour));
            } else {
                tours_.insert(tours_.begin() + position, dynamic_cast<ComposableTour*>(tour));
            }
        }

        void overrideTour(models::Tour* tour, unsigned long position) override {
            if (position < tours_.size()) {
                delete tours_[position];
                tours_[position] = dynamic_cast<ComposableTour*>(tour);
            }
        }

        unsigned long getNbTour() const override {
            return tours_.size();
        }

        models::Tour* getTour(unsigned t) const override {
            return t < tours_.size() ? tours_[t] : nullptr;
        }

        void copy(const models::Solution* p_solution) override {
            for (auto* tour : tours_) {
                delete tour;
            }
            tours_.clear();

            auto* other = dynamic_cast<const ComposableSolution*>(p_solution);
            if (other) {
                for (auto* tour : other->tours_) {
                    tours_.push_back(dynamic_cast<ComposableTour*>(tour->clone()));
                }
                totalCost_ = other->totalCost_;
                notserved = other->notserved;
            }
        }

        std::vector<models::Client*> getSequence() override {
            std::vector<models::Client*> seq;
            for (auto* tour : tours_) {
                for (unsigned long i = 0; i < tour->getNbClient(); ++i) {
                    seq.push_back(tour->getClient(i));
                }
            }
            return seq;
        }

        models::Solution* initFromSequence(Problem* problem,
                                           std::vector<models::Client*> sequence) override {
            // Minimal: create single-tour solution
            auto* sol = new ComposableSolution(problem);
            auto* tour = new ComposableTour(problem, 0);
            for (auto* client : sequence) {
                tour->_pushClient(client);
            }
            sol->pushTour(tour);
            return sol;
        }

        void print(std::ostream& out) override {
            out << "ComposableSolution:" << std::endl;
            out << "  Cost: " << totalCost_ << std::endl;
            out << "  Tours: " << tours_.size() << std::endl;
            for (size_t t = 0; t < tours_.size(); ++t) {
                out << "    Tour " << t << ": ";
                for (unsigned long i = 0; i < tours_[t]->getNbClient(); ++i) {
                    out << tours_[t]->getClient(i)->getID() << " ";
                }
                out << std::endl;
            }
            out << "  Unserved: " << notserved.size() << std::endl;
        }

#ifdef CPLEX_FOUND
        void getVarsVals(IloNumVarArray& vars, IloNumArray& vals) override {
            // Minimal: no-op
        }

        void constructFromIncumbent(IloCplex::HeuristicCallbackI* pCallback) override {
            // Minimal: no-op
        }

        void constructFromNode(IloCplex::HeuristicCallbackI* pCallback) override {
            // Minimal: no-op
        }
#endif

    private:
        std::vector<ComposableTour*> tours_;
        double totalCost_;
    };

} // namespace routing
