// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Solution.hpp"


vrp::models::Solution *
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
            if(! cost->isPossible()) break;
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
    solution->traveltime = cost;
    return solution;

}
