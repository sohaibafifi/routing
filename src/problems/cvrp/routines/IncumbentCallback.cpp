#include "IncumbentCallback.hpp"
#include "../Problem.hpp"
routing::callback::IncumbentCallback *CVRP::Problem::setIncumbentCallback(IloEnv &env)
{
    return new IncumbentCallback(env, this);
}

//void CVRP::IncumbentCallback::main()
//{
//    getEnv().out() << "Incumbent found of value " << getObjValue() << std::endl;
//    extractIncumbentSolution();
//    solution->print(getEnv().out());
////    bool valid = true;
////    std::vector<std::vector<unsigned int> > adjrel(problem->arcs.size());
////    for (unsigned int i = 0; i < problem->arcs.size(); i++) {
////        for (unsigned int j = 0; j < problem->arcs[i].size(); j++) {
////            if (getValue(problem->arcs[i][j]) > 0.5) {
////                adjrel[i].push_back(j);
////            }
////        }
////    }
////    std::vector<std::vector<unsigned int> > component;
////    Utilities::applyTarjanAlgorithm(adjrel, component);
////    valid = (component.size() == 0);
////    if (!valid) {
////        std::cout << "IncumbentCallback" << getSolutionSource() << " :";
////        std::cout << getObjValue() << " Rejected ====================================" << std::endl;
////        reject();
//    //    }
//}

void CVRP::IncumbentCallback::extractIncumbentSolution()
{
    solution = new Solution(static_cast<Problem*>(problem));
    unsigned t = 0, last = 0, served = 0;
    Tour * tour = new Tour(static_cast<Problem*>(problem), t);
    do {
        unsigned foundtime = 0;
        for (unsigned j = 0; j < problem->arcs.size(); ++j) {
            if (getValue(problem->arcs[last][j]) >= 1 - 1e-6) {
                if (last == 0 && foundtime < t) {
                    foundtime++;
                    continue;
                }
                if (j != 0) {
                    served++;
                    tour->pushClient(static_cast<CVRP::Client *>(problem->clients[j - 1]));
                }
                last = j;
                break;
            }
        }
        if (last == 0) {
            t++;
            solution->pushTour(tour);
            tour = new Tour(static_cast<Problem*>(problem), t);
        }

    }
    while (t < problem->vehicles.size() && served < problem->clients.size());
    solution->pushTour(tour);
    if(t == problem->vehicles.size()) delete tour;
    while(solution->getNbTour() < problem->vehicles.size()) {
        solution->pushTour(new Tour(static_cast<Problem*>(problem), solution->getNbTour()));
    }

}
