#include "ccvrp.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include "../../Utility.hpp"

void CCVRP::Problem::addVariables()
{
    CVRP::Problem::addVariables();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        arrival.push_back(IloNumVar(model.getEnv(), 0, IloInfinity, std::string("q_" + Utilities::itos(i)).c_str() ));
        model.add(arrival.back());
    }
}


void CCVRP::Problem::addSequenceConstraints()
{
    CVRP::Problem::addSequenceConstraints();
    for (unsigned i = 1; i <= clients.size(); ++i) {
        model.add(arrival[i] >= getDistance(*clients[i - 1], *getDepot()) );
        for (unsigned j = 1; j <= clients.size(); ++j) {
            if(i == j) continue;
            model.add(arrival[i]
                    +  routing::Problem::getDistance(*clients[i - 1], *clients[j - 1])
                    - arrival[j]
                    - 1e9 * (1 - arcs[i][j])  <= 0);
        }

    }
}

void CCVRP::Problem::addObjective()
{
    addTotalArrivalsObjective();
}


void CCVRP::Problem::addTotalArrivalsObjective()
{
    IloExpr obj(model.getEnv());
    for (unsigned i = 0; i <= clients.size(); ++i) {
        obj += arrival[i];
    }
    model.add(IloMinimize(model.getEnv(), obj));
}

CCVRP::Problem *CCVRP::Reader::readFile(std::string filepath){
    return new Problem(*CVRP::Reader::readFile(filepath));
}
