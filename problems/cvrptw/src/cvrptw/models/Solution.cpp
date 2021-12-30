// Copyright (c) 2021. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Solution.hpp"
#include "../Problem.hpp"


routing::models::Solution *
cvrptw::models::Solution::initFromSequence(routing::Problem *problem, std::vector<routing::models::Client *> sequence) {
    Solution *solution = dynamic_cast<Solution *>(problem->initializer()->initialSolution());
    std::vector<routing::Duration> labels;
    std::vector<long int> vs;
    for (int k = 0; k < sequence.size(); ++k) {
        labels.push_back(std::numeric_limits<routing::Duration>::infinity());
        vs.push_back(0);
    }
    for (int i = 0; i < sequence.size(); ++i) {
        routing::Duration distance = 0.0, length = 0.0;
        Tour tour(problem, 0);
        int j = i;
        do {
            auto cost = tour.evaluateInsertion(sequence[j], tour.getNbClient()); // TODO : check if this is enough
            if (!cost->isPossible()) break;
            if (i == j) {
                distance = problem->getDistance(*sequence[j], *dynamic_cast<Problem *>(problem)->getDepot());
                length = std::max(problem->getDistance(*sequence[j], *dynamic_cast<Problem *>(problem)->getDepot()),
                                  dynamic_cast<Client *>(sequence[j])->getEST());
            } else {
                distance =
                        distance - problem->getDistance(*sequence[j - 1], *dynamic_cast<Problem *>(problem)->getDepot())
                        + problem->getDistance(*sequence[j - 1], *sequence[j]);
                length = std::max(
                        length - problem->getDistance(*sequence[j - 1], *dynamic_cast<Problem *>(problem)->getDepot())
                        + problem->getDistance(*sequence[j - 1], *sequence[j]),
                        dynamic_cast<Client *>(sequence[j])->getEST());
            }
            if (
                    length > dynamic_cast<Client *>(sequence[j])->getLST()
                    ||
                    (length + dynamic_cast<Client *>(sequence[j])->getService() +
                     problem->getDistance(*sequence[j], *dynamic_cast<Problem *>(problem)->getDepot())) >
                    dynamic_cast<Depot *>(problem->depots[0])->getTwClose()
                    )
                break;

            tour.addClient(sequence[j], tour.getNbClient(), cost);
            distance = distance + problem->getDistance(*sequence[j], *dynamic_cast<Problem *>(problem)->getDepot());
            length = length + dynamic_cast<Client *>(sequence[j])->getService() +
                     problem->getDistance(*sequence[j], *dynamic_cast<Problem *>(problem)->getDepot());
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
        solution->addClient(tour, sequence.at(i), solution->getTour(tour)->getNbClient(), new routing::InsertionCost());
        if (sequence.at(delimiters.at(tour))->getID() == sequence.at(i)->getID()) {
            tour++;
        }
    }
    solution->update();
    solution->travelTime = cost;
    return solution;
}

#ifdef CPLEX

void cvrptw::models::Solution::getVarsVals(IloNumVarArray &vars, IloNumArray &vals) {
    vrp::models::Solution::getVarsVals(vars, vals);
    for (int t = 0; t < vrp::models::Solution::getNbTour(); ++t) {
        Tour *tour = dynamic_cast<Tour *>(tours[t]);
        routing::Demand consumption = 0;
        for (int v = 0; v < tour->getNbClient(); ++v) {
            Visit *visit = tour->getVisit(v);
            vars.add((dynamic_cast<cvrptw::Problem *>(problem))->start[visit->client->getID()]);
            vals.add(visit->getStart());

            consumption += visit->client->getDemand();
            vars.add((dynamic_cast<cvrp::Problem *>(problem))->consumption[visit->client->getID()]);
            vals.add(consumption);
        }
    }
}

#endif