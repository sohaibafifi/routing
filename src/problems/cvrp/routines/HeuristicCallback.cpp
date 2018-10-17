#include "HeuristicCallback.hpp"

#include "../../../Utility.hpp"
#include "../../../mtrand.hpp"

routing::callback::HeuristicCallback *CVRP::Problem::setHeuristicCallback(IloEnv &env)
{
    return new HeuristicCallback(env, this,
                                 new Constructor(),
                                 new Destructor());
}


void CVRP::HeuristicCallback::extractSolution()
{
    unsigned t = 0, last = 0;
    Tour * tour = new Tour(problem, t);
    do {
        unsigned foundtime = 0;
        for (unsigned j = 0; j < problem->arcs.size(); ++j) {
            if (getIncumbentValue(problem->arcs[last][j]) >= 1 - 1e-6) {
                if (last == 0 && foundtime < t) {
                    foundtime++;
                    continue;
                }
                if (j != 0) {
                    solution->notserved.erase(std::remove(solution->notserved.begin(), solution->notserved.end(), static_cast<CVRP::Client *>(problem->clients[j - 1])),
                                              solution->notserved.end());
                    tour->pushClient(static_cast<CVRP::Client *>(problem->clients[j - 1]));
                }
                last = j;
                break;
            }
        }
        if (last == 0) {
            t++;
            solution->pushTour(tour);
            tour = new Tour(problem, t);
        }

    }
    while (t < problem->vehicles.size() && solution->notserved.size() > 0);
    solution->pushTour(tour);
    if(t == problem->vehicles.size()) delete tour;
    while(solution->getNbTour() < problem->vehicles.size()) {
        solution->pushTour(new Tour(problem, solution->getNbTour()));
    }
}

void CVRP::HeuristicCallback::main()
{
    initSolution();
    routing::callback::HeuristicCallback::main();
    delete solution;
}
