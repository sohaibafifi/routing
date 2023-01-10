// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Solution.hpp"


routing::models::Solution *
vrp::models::Solution::initFromSequence(routing::Problem *problem, std::vector<routing::models::Client *> sequence) {
    Solution *solution = dynamic_cast<Solution *>(problem->initializer()->initialSolution());
    std::vector<routing::Duration> labels;
    std::vector<long int> vs;
    for (int k = 0; k < sequence.size(); ++k) {
        labels.push_back(std::numeric_limits<routing::Duration>::infinity());
        vs.push_back(0);
    }
    for (int i = 0; i < sequence.size(); ++i) {
        routing::Duration distance = 0.0;
        Tour tour(problem, 0);
        int j = i;
        do {
            auto cost = tour.evaluateInsertion(sequence[j], tour.getNbClient());
            if (!cost->isPossible()) break;
            tour._pushClient(sequence[j]);
            if (i == j) {
                distance = problem->getDistance(*sequence[j], *dynamic_cast<Problem *>(problem)->getDepot());
            } else {
                distance =
                        distance - problem->getDistance(*sequence[j - 1], *dynamic_cast<Problem *>(problem)->getDepot())
                        + problem->getDistance(*sequence[j - 1], *sequence[j]);
            }
            distance = distance + problem->getDistance(*sequence[j], *dynamic_cast<Problem *>(problem)->getDepot());
            routing::Duration lastCost = (i == 0) ? 0.0 : labels.at(i - 1);
            if (lastCost + distance < labels.at(j)) {
                labels[j] = lastCost + distance;
                vs[j] = i - 1;
            }
            j++;
        } while (j < sequence.size());


    }
    routing::Duration cost = labels.at(sequence.size() - 1);
    std::vector<int> delimiters;
    int minnode = vs.at(sequence.size() - 1);
    while (minnode != -1) {
        delimiters.push_back(minnode);
        minnode = vs.at(minnode);
    }
    std::reverse(delimiters.begin(), delimiters.end());
    delimiters.push_back(sequence.size() - 1);
    int tour = 0;
    for (int i = 0; i < sequence.size(); i++) {

        solution->pushClient(tour, sequence.at(i));
        if (sequence.at(delimiters.at(tour))->getID() == sequence.at(i)->getID()) {

            tour++;
        }
    }
    solution->travelTime = cost;
    return solution;

}

#ifdef CPLEX_FOUND

void vrp::models::Solution::constructFromIncumbent(IloCplex::HeuristicCallbackI *pCallback) {
    std::vector<bool> inserted(dynamic_cast<Problem *>(problem)->clients.size() + 1, false);
    for (int t = 0; t < problem->vehicles.size(); ++t) {
        unsigned departure = 0;
        do {
            for (int i = 0; i <= dynamic_cast<Problem *>(problem)->clients.size(); ++i) {
                if (i > 0 && inserted[i])
                    continue;
                if (pCallback->getIncumbentValue(dynamic_cast<Problem *>(problem)->arcs[departure][i]) > 0.5
                    && (
                            i == 0
                            //||
                            //pCallback->getIncumbentValue(dynamic_cast<Problem *>(problem)->affectation[i][t]) > 0.5
                    )
                        ) {
                    departure = i;
                    if (departure == 0) break;
                    inserted[i] = true;
                    this->addClient(t, dynamic_cast<Problem *>(problem)->clients[i - 1],
                                    this->getTour(t)->getNbClient(), new routing::InsertionCost());
                }
            }
        } while (departure != 0);
    }
    this->update();
}


void vrp::models::Solution::constructFromNode(IloCplex::HeuristicCallbackI *pCallback) {
    std::vector<bool> inserted(dynamic_cast<Problem *>(problem)->clients.size() + 1, false);
    for (int t = 0; t < problem->vehicles.size(); ++t) {
        unsigned departure = 0;
        do {
            for (int i = 0; i <= dynamic_cast<Problem *>(problem)->clients.size(); ++i) {
                if (i > 0 && inserted[i])
                    continue;
                if (pCallback->getValue(dynamic_cast<Problem *>(problem)->arcs[departure][i]) > 0.99
                    && (
                            i == 0
                            //||
                            //pCallback->getIncumbentValue(dynamic_cast<Problem *>(problem)->affectation[i][t]) > 0.5
                    )
                        ) {
                    departure = i;
                    if (departure == 0) break;
                    inserted[i] = true;
                    this->addClient(t, dynamic_cast<Problem *>(problem)->clients[i - 1],
                                    this->getTour(t)->getNbClient(), new routing::InsertionCost());
                }
//                else {
//                    try {
//                        if (
//                                i < dynamic_cast<Problem *>(problem)->affectation.size()
//                                && t < dynamic_cast<Problem *>(problem)->affectation[i].size()
//                                && pCallback->getValue(dynamic_cast<Problem *>(problem)->affectation[i][t]) > 0.99
//                                ) {
//                            departure = i;
//                            if (departure == 0) break;
//                            inserted[i] = true;
//                            this->addClient(t, dynamic_cast<Problem *>(problem)->clients[i - 1],
//                                            this->getTour(t)->getNbClient(), new routing::InsertionCost());
//                        }
//                    } catch (IloCplex::ControlCallbackI::PresolvedVariableException &e) {
//                        continue;
//                    }
//
//                }
            }
        } while (departure != 0);
    }
    this->update();
}


void vrp::models::Solution::getVarsVals(IloNumVarArray &vars, IloNumArray &vals) {
    vrp::Problem *problem = dynamic_cast<vrp::Problem * >(this->problem);
    std::vector<std::vector<IloBool> > values(problem->arcs.size());
    std::vector<std::vector<IloBool> > affectation(problem->arcs.size());
    std::vector<unsigned> order(problem->arcs.size());
    for (unsigned j = 0; j < problem->arcs.size(); ++j) {
        values[j] = std::vector<IloBool>(problem->arcs.size(), IloFalse);
    }
    for (unsigned j = 0; j < problem->arcs.size(); ++j) {
        affectation[j] = std::vector<IloBool>(problem->vehicles.size(), IloFalse);
    }
    for (unsigned k = 0; k < this->getNbTour(); ++k) {
        unsigned last = 0;
        for (unsigned i = 0; i < this->getTour(k)->getNbClient(); ++i) {
            values[last][this->getTour(k)->getClient(i)->getID()] = IloTrue;
            last = this->getTour(k)->getClient(i)->getID();
            affectation[last][k] = IloTrue;


            order[this->getTour(k)->getClient(i)->getID()] = i;
        }
        values[last][0] = IloTrue;
    }

    for (unsigned i = 0; i < problem->arcs.size(); ++i) {
        for (unsigned j = 0; j < problem->arcs.size(); ++j) {
            if (i == j) continue;
            vars.add(problem->arcs[i][j]);
            vals.add(values[i][j]);

        }
        for (unsigned k = 0; k < affectation[i].size(); ++k) {
            //vars.add(problem->affectation[i][k]);
            //vals.add(affectation[i][k]);
        }
        if (i > 0) {
            if (i < problem->order.size() && problem->order[i].getImpl() != nullptr) {
                vars.add(problem->order[i]);
                vals.add(order[i]);
            }
        }
    }
}


#endif


void vrp::models::Solution::addClient(unsigned long index_tour, routing::models::Client *client, unsigned long position,
                                      routing::InsertionCost *cost) {
    routing::Duration oldCost = this->getTour(index_tour)->getTravelTime();
    routing::models::Solution::addClient(index_tour, client, position, cost);
    routing::Duration delta = this->getTour(index_tour)->getTravelTime() - oldCost;
    this->travelTime += delta;
}

void vrp::models::Solution::pushClient(unsigned long index_tour, routing::models::Client *client) {
    routing::Duration oldCost = this->getTour(index_tour)->getTravelTime();
    routing::models::Solution::pushClient(index_tour, client);
    routing::Duration delta = this->getTour(index_tour)->getTravelTime() - oldCost;
    this->travelTime += delta;
}

void vrp::models::Solution::removeClient(unsigned long index_tour, unsigned long position) {
    routing::Duration oldCost = this->getTour(index_tour)->getTravelTime();
    routing::models::Solution::removeClient(index_tour, position);
    routing::Duration delta = this->getTour(index_tour)->getTravelTime() - oldCost;
    this->travelTime += delta;
}

void vrp::models::Solution::overrideTour(routing::models::Tour *tour, unsigned long position) {
    travelTime -= dynamic_cast<Tour *>( tours.at(position))->getTravelTime();
    tours.at(position) = dynamic_cast<Tour *>(tour);
    travelTime += dynamic_cast<Tour *>( tour )->getTravelTime();
}

void vrp::models::Solution::addTour(routing::models::Tour *tour, unsigned long position) {
    tours.insert(tours.begin() + position, dynamic_cast<Tour *>(tour));
    travelTime += dynamic_cast<Tour *>( tour )->getTravelTime();
}

void vrp::models::Solution::pushTour(routing::models::Tour *tour) {
    addTour(tour, getNbTour());
}

void vrp::models::Solution::copy(const routing::models::Solution *p_solution) {
    this->travelTime = dynamic_cast<const Solution *>(p_solution)->travelTime;
    this->notserved.clear();
    for (auto i: p_solution->notserved) {
        this->notserved.push_back(i);
    }
    this->tours.clear();
    for (unsigned int j = 0; j < dynamic_cast<const Solution *>(p_solution)->tours.size(); ++j) {
        this->tours.push_back(dynamic_cast<const Solution *>(p_solution)->getTour(j)->clone());
    }
}

void vrp::models::Solution::update() {
    travelTime = 0;
    for (int t = 0; t < getNbTour(); ++t) {
        getTour(t)->update();
        travelTime += getTour(t)->getTravelTime();
    }
}

void vrp::models::Solution::print(std::ostream &out) {
    out << "solution cost " << getCost() << std::endl;
    for (unsigned t = 0; t < getNbTour(); t++) {
        out << "tour " << t << " : ";
        for (unsigned i = 0; i < this->getTour(t)->getNbClient(); ++i) {
            out << this->getTour(t)->getClient(i)->getID() << " ";
        }
        out << "[" << this->getTour(t)->getHash() << "]" << std::endl;
    }
}

std::vector<routing::models::Client *> vrp::models::Solution::getSequence() {
    std::vector<routing::models::Client *> sequence;
    for (int t = 0; t < tours.size(); ++t) {
        for (int i = 0; i < getTour(t)->getNbClient(); ++i) {
            sequence.push_back(getTour(t)->getClient(i));
        }
    }
    return sequence;
}
