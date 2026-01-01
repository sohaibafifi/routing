// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Model.hpp"
#include "models/Tour.hpp"
#include "Entity.hpp"
#include "core/cache/RouteCache.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>

#ifdef CPLEX_FOUND
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <ilcplex/ilocplexi.h>
#pragma GCC diagnostic pop
#endif

namespace routing {

    // Forward declaration
    class Problem;

    /**
     * @brief Tour implementation for composable problems
     *
     * This class stores a sequence of clients assigned to a vehicle.
     */
    class Tour : public models::Tour {
    public:
        Tour(Problem* p_problem, unsigned vehicleID);

        void update() override;

        void pushClient(models::Client* client, InsertionCost* cost) override {
            clients_.push_back(client);
            if (cost) cost_ += cost->getDelta();
        }

        InsertionCost* _pushClient(models::Client* client) override {
            clients_.push_back(client);
            return new InsertionCost(0, true);
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
            cache_.invalidate();
        }

        models::Client* getClient(unsigned long pos) const override {
            return pos < clients_.size() ? clients_[pos] : nullptr;
        }

        unsigned long getNbClient() const override {
            return clients_.size();
        }

        models::Tour* clone() const override;

        InsertionCost* evaluateInsertion(models::Client* client, unsigned long position) override;

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

        // ========== Cache Support ==========

        /// Get mutable cache reference
        RouteCache& getCache() { return cache_; }

        /// Get const cache reference
        const RouteCache& getCache() const { return cache_; }

        /// Check if cache is valid
        bool isCacheValid() const { return cache_.isValid(); }

        /// Mark cache as invalid (will be rebuilt on next ensureCache())
        void invalidateCache() { cache_.invalidate(); }

        /// Ensure cache is populated (lazy initialization)
        /// This is declared here but implemented in Problem.hpp after Problem is defined
        void ensureCache() const;

    private:
        std::vector<models::Client*> clients_;
        double cost_;
        mutable RouteCache cache_;  ///< Cached state for incremental evaluation
    };

    /**
     * @brief Solution implementation for composable problems
     *
     * This class represents a complete solution consisting of multiple tours.
     */
    class Solution : public Model {
    protected:
        Problem* problem_;
        double penaltyCost_;

    public:
        explicit Solution(Problem* p_problem)
            : problem_(p_problem), totalCost_(0), penaltyCost_(0) {}

        Solution(const Solution& other)
            : problem_(other.problem_), totalCost_(other.totalCost_), penaltyCost_(other.penaltyCost_) {
            copy(&other);
        }

        virtual ~Solution() {
            for (auto* tour : tours_) {
                delete tour;
            }
        }

        virtual void update() {
            totalCost_ = 0;
            for (auto* tour : tours_) {
                tour->update();
                totalCost_ += tour->getCost();
            }
        }

        virtual double getCost() {
            return totalCost_ + penaltyCost_;
        }

        virtual void setPenalty(double penalty) {
            penaltyCost_ = penalty;
        }

        virtual double getPenalty() const {
            return penaltyCost_;
        }

        virtual void pushTour(Tour* tour) {
            tours_.push_back(tour);
        }

        virtual void addTour(Tour* tour, unsigned long position) {
            if (position >= tours_.size()) {
                tours_.push_back(tour);
            } else {
                tours_.insert(tours_.begin() + position, tour);
            }
        }

        virtual void overrideTour(Tour* tour, unsigned long position) {
            if (position < tours_.size()) {
                delete tours_[position];
                tours_[position] = tour;
            }
        }

        virtual unsigned long getNbTour() const {
            return tours_.size();
        }

        virtual Tour* getTour(unsigned t) const {
            return t < tours_.size() ? tours_[t] : nullptr;
        }

        virtual void copy(const Solution* p_solution) {
            for (auto* tour : tours_) {
                delete tour;
            }
            tours_.clear();

            if (p_solution) {
                for (size_t i = 0; i < p_solution->tours_.size(); ++i) {
                    tours_.push_back(dynamic_cast<Tour*>(p_solution->tours_[i]->clone()));
                }
                totalCost_ = p_solution->totalCost_;
                penaltyCost_ = p_solution->penaltyCost_;
                notserved = p_solution->notserved;
            }
        }

        virtual std::vector<models::Client*> getSequence() {
            std::vector<models::Client*> seq;
            for (auto* tour : tours_) {
                for (unsigned long i = 0; i < tour->getNbClient(); ++i) {
                    seq.push_back(tour->getClient(i));
                }
            }
            return seq;
        }

        virtual Solution* initFromSequence(Problem* problem, std::vector<models::Client*> sequence);

        virtual void print(std::ostream& out) {
            out << "Solution:" << std::endl;
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

        virtual Solution* clone() const {
            auto* copy = new Solution(problem_);
            copy->copy(this);
            return copy;
        }

        Problem* getProblem() { return problem_; }
        std::vector<models::Client*> notserved;

        Solution& operator=(const Solution& p_solution) {
            if (this != &p_solution) {
                this->copy(&p_solution);
            }
            return *this;
        }

        virtual void pushClient(unsigned long index_tour, models::Client* client) {
            this->notserved.erase(
                std::remove(this->notserved.begin(), this->notserved.end(), client),
                this->notserved.end());
            this->getTour(index_tour)->_pushClient(client);
        }

        virtual void addClient(unsigned long index_tour, models::Client* client,
                               unsigned long position, InsertionCost* cost) {
            assert(cost->isPossible());
            this->notserved.erase(
                std::remove(this->notserved.begin(), this->notserved.end(), client),
                this->notserved.end());
            this->getTour(index_tour)->addClient(client, position, cost);
        }

        virtual void removeClient(unsigned long index_tour, unsigned long position) {
            this->notserved.push_back(this->getTour(index_tour)->getClient(position));
            this->getTour(index_tour)->removeClient(position);
        }

#ifdef CPLEX_FOUND
        virtual void getVarsVals(IloNumVarArray& vars, IloNumArray& vals) {
            // Minimal: no-op
        }

        virtual void constructFromIncumbent(IloCplex::HeuristicCallbackI* pCallback) {
            // Minimal: no-op
        }

        virtual void constructFromNode(IloCplex::HeuristicCallbackI* pCallback) {
            // Minimal: no-op
        }
#endif

    private:
        std::vector<Tour*> tours_;
        double totalCost_;
    };

    // Backward compatibility aliases (deprecated)
    using ComposableTour = Tour;
    using ComposableSolution = Solution;

} // namespace routing
