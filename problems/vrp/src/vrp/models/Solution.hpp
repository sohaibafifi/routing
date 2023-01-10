// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#pragma once

#include <core/data/models/Solution.hpp>
#include "../Problem.hpp"
#include "Tour.hpp"

namespace vrp {
    namespace models {

        class Solution : public routing::models::Solution {
        public:
            explicit Solution(routing::Problem *p_problem)
                    : routing::models::Solution(p_problem),
                      travelTime(0),
                      tours(std::vector<Tour *>()) {
                while (this->getNbTour() < this->getProblem()->vehicles.size()) {
                    this->pushTour(this->problem->initializer()->initialTour(this->getNbTour()));
                }

            }

            Solution(const Solution &p_solution) :
                    routing::models::Solution(p_solution),
                    travelTime(p_solution.travelTime) {
                this->copy(&p_solution);
            }


            void update() override;

            void copy(const routing::models::Solution *p_solution) override;

            double getCost() override {
                return travelTime;
            }


            void pushTour(routing::models::Tour *tour) override;

            void addTour(routing::models::Tour *tour, unsigned long position) override;

            void overrideTour(routing::models::Tour *tour, unsigned long position) override;

            void removeClient(unsigned long index_tour, unsigned long position) override;

            void pushClient(unsigned long index_tour, routing::models::Client *client) override;

            void addClient(unsigned long index_tour, routing::models::Client *client, unsigned long position,
                           routing::InsertionCost *cost) override;

            unsigned long getNbTour() const override {
                return tours.size();
            }

#ifdef CPLEX_FOUND

            void getVarsVals(IloNumVarArray &vars, IloNumArray &vals) override;

            void constructFromIncumbent(IloCplex::HeuristicCallbackI *pCallback) override;
            void constructFromNode(IloCplex::HeuristicCallbackI *pCallback) override;
#endif

            void print(std::ostream &out) override;

            Tour *getTour(unsigned t) const override {
                return tours.at(t);
            }

            std::vector<routing::models::Client *> getSequence() override;

            routing::Duration travelTime;

        protected :
            std::vector<Tour *> tours;

            routing::models::Solution *
            initFromSequence(routing::Problem *problem, std::vector<routing::models::Client *> sequence) override;

        };

    }
}