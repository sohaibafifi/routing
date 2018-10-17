#include "Problem.hpp"
#include "../../Utility.hpp"
#include "models/Client.hpp"
#include "models/Depot.hpp"
#include "models/Vehicle.hpp"

IloModel CVRP::Problem::generateModel(IloEnv &env)
{
    this->model = IloModel(env);
    this->addVariables();
    this->addRoutingConstraints();
    this->addSequenceConstraints();

    this->addAffectationConstraints();
    this->addCapacityConstraints();

    this->addObjective();
    return this->model;
}


void CVRP::Problem::addVariables()
{
    routing::Problem::addVariables();
    for (unsigned i = 0; i <= clients.size(); ++i) {
        if (i == 0)
            consumption.push_back(IloNumVar(model.getEnv(),
                                            0, 0,
                                            std::string("q_" + Utilities::itos(i)).c_str()));
        else

            consumption.push_back(IloNumVar(model.getEnv(),
                                            static_cast<Client *>(clients[i - 1])->getDemand(),
                                  static_cast<Vehicle *>(vehicles[0])->getCapacity(),
                    std::string("q_" + Utilities::itos(i)).c_str()));
        model.add(consumption.back());
    }
}

void CVRP::Problem::addSequenceConstraints()
{
    for (auto i = 1; i <= clients.size(); ++i) {
        for (auto j = 1; j <= clients.size(); ++j) {
            if (i == j) continue;
            model.add(consumption[i]
                      + static_cast<Client *>(clients[j - 1])->getDemand()
                    <= consumption[j]
                    + (static_cast<Vehicle *>(vehicles[0])->getCapacity()
                    + static_cast<Client *>(clients[j - 1])->getDemand()) * (1 - arcs[i][j]));
        }

    }
}

void CVRP::Problem::addCapacityConstraints()
{
    for (auto k = 0; k < vehicles.size(); ++k) {
        IloExpr expr(model.getEnv());
        for (auto i = 1; i <= clients.size(); ++i) {
            expr += static_cast<Client *>(clients[i - 1])->getDemand() * affectation[i][k];
        }
        model.add(expr <= static_cast<Vehicle *>(vehicles[k])->getCapacity());
    }
}

void CVRP::Problem::addObjective()
{
    addTotalDistanceObjective();
}

void CVRP::Problem::addTotalDistanceObjective()
{
    obj = IloExpr(model.getEnv());
    for (int i = 0; i < clients.size(); ++i) {
        obj += getDistance(*clients[i], *getDepot()) * arcs[i + 1][0];
        obj += getDistance(*clients[i], *getDepot()) * arcs[0][i + 1];
        for (int j = 0; j < clients.size(); ++j) {
            obj += routing::Problem::getDistance(*clients[i], *clients[j]) * arcs[i + 1][j + 1];
        }
    }
    model.add(IloMinimize(model.getEnv(), obj));
}

CVRP::Depot *CVRP::Problem::getDepot()
{    
    return static_cast<Depot*>( depots[0] );
}

void CVRP::Problem::setDepot(CVRP::Depot *depot)
{
    depots.clear();
    depots.push_back(depot);
}
