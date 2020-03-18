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
                      traveltime(0),
                      tours(std::vector<Tour *>()) {
                while (this->getNbTour() < this->getProblem()->vehicles.size()) {
                    this->pushTour(this->problem->initializer()->initialTour(this->getNbTour()));
                }

            }

            Solution(const Solution &p_solution) :
                    routing::models::Solution(p_solution) ,
                    traveltime(p_solution.traveltime){
                this->copy(&p_solution);
            }


            void update() override {}

            void copy(const routing::models::Solution *p_solution) override {
                this->traveltime = dynamic_cast<const Solution *>(p_solution)->traveltime;
                this->notserved.clear();
                for (auto i : p_solution->notserved) {
                    this->notserved.push_back(i);
                }
                this->tours.clear();
                for (unsigned int j = 0; j < dynamic_cast<const Solution *>(p_solution)->tours.size(); ++j) {
                    this->tours.push_back(dynamic_cast<const Solution *>(p_solution)->getTour(j)->clone());
                }
            }

            double getCost() override {
                return traveltime;
            }


            void pushTour(routing::models::Tour *tour) override {
                addTour(tour, getNbTour());
            }

            void addTour(routing::models::Tour *tour, unsigned long position) override {
                tours.insert(tours.begin() + position, dynamic_cast<Tour *>(tour));
                traveltime += dynamic_cast<Tour *>( tour )->getTravelTime();
            }

            void overrideTour(routing::models::Tour *tour, unsigned long position) override {
                traveltime -= dynamic_cast<Tour *>( tours.at(position))->getTravelTime();
                tours.at(position) = dynamic_cast<Tour *>(tour);
                traveltime += dynamic_cast<Tour *>( tour )->getTravelTime();
            }

            void removeClient(unsigned long index_tour, unsigned long position) override {
                routing::Duration oldCost = this->getTour(index_tour)->getTravelTime();
                routing::models::Solution::removeClient(index_tour, position);
                routing::Duration delta = this->getTour(index_tour)->getTravelTime() - oldCost;
                this->traveltime += delta;
            }

            void pushClient(unsigned long index_tour, routing::models::Client *client) override {
                routing::Duration oldCost = this->getTour(index_tour)->getTravelTime();
                routing::models::Solution::pushClient(index_tour, client);
                routing::Duration delta = this->getTour(index_tour)->getTravelTime() - oldCost;
                this->traveltime += delta;
            }

            void addClient(unsigned long index_tour, routing::models::Client *client, unsigned long position) override {
                routing::Duration oldCost = this->getTour(index_tour)->getTravelTime();
                routing::models::Solution::addClient(index_tour, client, position);
                routing::Duration delta = this->getTour(index_tour)->getTravelTime() - oldCost;
                this->traveltime += delta;
            }

            unsigned long getNbTour() const override {
                return tours.size();
            }

#ifdef CPLEX
            void getVarsVals(IloNumVarArray &vars, IloNumArray &vals) override {
                vrp::Problem *problem = dynamic_cast<vrp::Problem * >(this->problem);
                std::vector<std::vector<IloBool> > values(problem->arcs.size());
                std::vector<std::vector<IloBool> > affectation(problem->affectation.size());
                for (unsigned j = 0; j < problem->arcs.size(); ++j) {
                    values[j] = std::vector<IloBool>(problem->arcs.size(), IloFalse);
                    affectation[j] = std::vector<IloBool>(problem->affectation[j].size(), IloFalse);
                }
                for (unsigned k = 0; k < this->getNbTour(); ++k) {
                    unsigned last = 0;
                    for (unsigned i = 0; i < this->getTour(k)->getNbClient(); ++i) {
                        values[last][this->getTour(k)->getClient(i)->getID()] = IloTrue;
                        last = this->getTour(k)->getClient(i)->getID();
                        affectation[last][k] = IloTrue;
                    }
                    values[last][0] = IloTrue;
                }

                for (unsigned i = 0; i < problem->arcs.size(); ++i) {
                    for (unsigned j = 0; j < problem->arcs.size(); ++j) {
                        if (i == j) continue;
//                        if (values[i][j])
                        {
                            vars.add(problem->arcs[i][j]);
                            vals.add(values[i][j]);

                        }
                    }

                    for (unsigned k = 0; k < problem->affectation[i].size(); ++k) {
//                        if (affectation[i][k])
                        {
                            vars.add(problem->affectation[i][k]);
                            vals.add(affectation[i][k]);
                        }
                    }
                }
            }
#endif

            void print(std::ostream &out) override {
                out << "solution cost " << getCost() << std::endl;
                for (unsigned t = 0; t < getNbTour(); t++) {
                    out << "tour " << t << " : ";
                    for (unsigned i = 0; i < this->getTour(t)->getNbClient(); ++i) {
                        out << this->getTour(t)->getClient(i)->getID() << " ";
                    }
                    out << "[" <<this->getTour(t)->getHash() << "]" << std::endl;
                }
            }

            Tour *getTour(unsigned t) const override {
                return tours.at(t);
            }

            std::vector<routing::models::Client *> getSequence() override {
                std::vector<routing::models::Client *> sequence;
                for (int t = 0; t < tours.size(); ++t) {
                    for (int i = 0; i < getTour(t)->getNbClient(); ++i) {
                        sequence.push_back(getTour(t)->getClient(i));
                    }
                }
                return sequence;
            }

            routing::Duration traveltime;
            
        protected :
            std::vector<Tour *> tours;

            Solution *
            initFromSequence(routing::Problem *problem, std::vector<routing::models::Client *> sequence) override;

        };

    }
}