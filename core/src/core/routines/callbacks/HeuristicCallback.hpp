#pragma once

#include <ilcplex/ilocplexi.h>

#include "../../data/Problem.hpp"
#include "core/routines/operators/Generator.hpp"
#include "core/routines/operators/Constructor.hpp"
#include "core/routines/operators/Destructor.hpp"
#include "core/routines/operators/Diver.hpp"

namespace routing {

    namespace callback {
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
                    neighbors(p_neighbors) {
                InitialFound = false;
            }

            ~HeuristicCallback() = default;

        protected:
            bool InitialFound;
            Problem *problem;
            routing::Generator *generator;
            routing::Diver *diver;
            std::vector<routing::Neighborhood *> neighbors;

            virtual void main() {
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
                        getEnv().out() << "Local search ... " << std::flush;
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
                        }
                        getEnv().out() << solution->getCost() << " [Incumbent = " << getIncumbentObjValue() << "]"
                                       << std::endl;
                        if (!improved) return;

                    }
                } else {
                    getEnv().out() << "Construct solution from scratch ... " << std::flush;
                    this->solution = generator->generate();
                    if (solution == nullptr) {
                        getEnv().out() << std::endl;
                        return;
                    }
                    getEnv().out() << solution->getCost() << " [Incumbent = " << getIncumbentObjValue() << "]"
                                   << std::endl;
                }

                if (!hasIncumbent() || solution->getCost() < getIncumbentObjValue() - 1e-9) {
                    IloNumVarArray vars(getEnv());
                    IloNumArray vals(getEnv());
                    solution->getVarsVals(vars, vals);

                    for (unsigned i = 0; i < vars.getSize(); ++i) setBounds(vars[i], vals[i], vals[i]);

                    solve();
                    if (hasIncumbent())
                        getEnv().out() << "CVRPHeuristicCallback from " << getIncumbentObjValue() << " to "
                                       << solution->getCost()
                                       << " -  " << getObjValue() << "  " << getCplexStatus() << std::endl;
                    InitialFound = (getCplexStatus() == CPX_STAT_OPTIMAL);
                    for (unsigned i = 0; i < vars.getSize(); ++i) vals[i] = getValue(vars[i]);
                    setSolution(vars, vals, getObjValue());
                    vals.end();
                    vars.end();
                }
            }

            IloCplex::CallbackI *duplicateCallback() const {
                throw new std::logic_error("Not implemented");
            }

            models::Solution *solution;

            virtual void extractSolution() {
                throw new std::logic_error("Not implemented");
            }

            virtual bool shouldDive() {
                return false;
            }

            virtual routing::models::Solution *extractPartialSolution(routing::Problem *problem) {
                return nullptr;
            }
        };
    }
}
