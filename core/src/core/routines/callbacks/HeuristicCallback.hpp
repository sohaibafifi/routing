// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <ilcplex/ilocplexi.h>

#include <utility>

#include "../../data/Problem.hpp"
#include "core/routines/operators/Generator.hpp"
#include "core/routines/operators/Constructor.hpp"
#include "core/routines/operators/Destructor.hpp"
#include "core/routines/operators/Diver.hpp"

namespace routing::callback {
    class HeuristicCallback
            : public IloCplex::HeuristicCallbackI {
    public:
        HeuristicCallback(IloEnv env, Problem *_problem,
                          routing::Generator *p_generator,
                          routing::Diver *p_diver,
                          std::vector<routing::Neighborhood *> p_neighbors)
                :
                IloCplex::HeuristicCallbackI(env),
                problem(_problem),
                generator(p_generator),
                diver(p_diver),
                neighbors(std::move(p_neighbors)), incumbent_looked(0) {
        }

        ~HeuristicCallback() override = default;

    protected:
        Problem *problem;
        routing::Generator *generator;
        routing::Diver *diver;
        std::vector<routing::Neighborhood *> neighbors;
        double incumbent_looked;

        void main() override {
            if (hasIncumbent()) {

                if (shouldDive()) {
                    solution = this->extractPartialSolution(problem);
                    if (solution == nullptr) return;
                    if (diver->dive(solution))
                        getEnv().out() << solution->getCost() << " [Incumbent = " << getIncumbentObjValue() << "]"
                                       << std::endl;
                    else {
                        getEnv().out() << "Not found [Incumbent = " << getIncumbentObjValue() << "]" << std::endl;
                        return;
                    }
                } else {
                    // if (incumbent_looked == getIncumbentObjValue()) return;
                    incumbent_looked = getIncumbentObjValue();
                    this->extractSolution();
                    std::random_device rd;
                    bool improved = false;
                    std::vector<bool> run(neighbors.size(), false);
                    while (std::find(run.begin(), run.end(), false) != run.end()) {
                        unsigned long i = 0;
                        do { i = rd() % run.size(); } while (run[i]);

                        if (neighbors[i]->look(solution)) {
                            improved = true;
                            run = std::vector<bool>(neighbors.size(), false);
                        } else {
                            run[i] = true;
                        }
                        break; // FIXME : add a param to choose between two modes :
                        // One to execute only one LS at once
                        // and the other uses the loop
                    }
                    if (!improved) return;

                }
            } else {
                getEnv().out() << getCurrentNodeDepth() << " Construct solution from scratch ... " << std::flush;
                getEnv().out() << " using  " << typeid(generator->getConstructor()).name() << "  " << std::flush;

                this->solution = generator->generate();
                if (solution == nullptr) {
                    getEnv().out() << std::endl;
                    return;
                }
                getEnv().out() << solution->getCost() << std::endl;
            }

            if (!hasIncumbent() || solution->getCost() < getIncumbentObjValue() - 1e-9) {
                try {
                    IloNumVarArray all_vars(getEnv());
                    IloNumArray all_vals(getEnv());
                    solution->getVarsVals(all_vars, all_vals);

                    for (unsigned i = 0; i < all_vars.getSize(); ++i) {
                        try {
                            setBounds(all_vars[i], std::floor(all_vals[i]), std::ceil(all_vals[i]));
                        } catch (PresolvedVariableException &e) {
                            continue;
                        }
                    }
                    // FIXME : the solution is not always accepted
                    auto solved = solve();
                    if (solved) {
                        getEnv().out() << "HeuristicCallback from " << getIncumbentObjValue() << " to "
                                       << solution->getCost()
                                       << " - " << getObjValue() << "  " << getCplexStatus() << std::endl;
                        IloNumVarArray vars(getEnv());
                        IloNumArray vals(getEnv());
                        for (unsigned i = 0; i < vars.getSize(); ++i) {
                            try {
                                // keep only not removed variables because of the pre-solve
                                if (getFeasibility(all_vars[i]) == this->Feasible) {
                                    vars.add(all_vars[i]);
                                    vals.add(getValue(vars[i]));
                                }
                            } catch (PresolvedVariableException &e) {
                                continue;
                            }
                        }
                        setSolution(vars, vals, getObjValue());
                        vals.end();
                        vars.end();
                    }
                    all_vars.end();
                    all_vals.end();
                } catch (IloException &exception) {
                    getEnv().warning() << exception.getMessage() << std::endl;
                }


            }
        }

        IloCplex::CallbackI *duplicateCallback() const override {
            return new(getEnv()) HeuristicCallback(*this);
        }

        models::Solution *solution{};
        std::vector<double> starts;

        virtual void extractSolution() {
            this->solution = this->generator->initialSolution();
            // TODO : transfer information about arc feasibility using pre-solve
            this->solution->constructFromModel(this);
        }

        virtual bool shouldDive() {
            // extract solution should work
            return false;
        }

        virtual routing::models::Solution *extractPartialSolution(routing::Problem *problem) {
            return nullptr;
        }
    };
}

