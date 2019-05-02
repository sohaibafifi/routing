#include "toptw.hpp"


TOPTW::Problem *TOPTW::Reader::readFile(std::string filepath)
{
    CVRPTW::Problem * cvrptw_problem = CVRPTW::Reader().readFile(filepath);
    TOPTW::Problem * problem = new TOPTW::Problem (*cvrptw_problem);
    for (unsigned i = 0; i < problem->clients.size(); ++i) {
        CVRPTW::Client* old = static_cast<CVRPTW::Client*>(problem->clients[i]);
        problem->clients[i] = new  Client(*old);
        delete old;
        static_cast<Client*>(problem->clients[i])->demand = 0;
    }
    for (unsigned k = 0; k < problem->vehicles.size(); ++k) {
        static_cast<CVRPTW::Vehicle*>(problem->vehicles[k])->capacity = IloInfinity;
    }
    return problem;
}

void TOPTW::Problem::addProfitsObjective()
{
    IloExpr obj(model.getEnv());
    for (unsigned i = 0; i <= clients.size(); ++i) {
        for (unsigned j = 1; j <= clients.size(); ++j) {
            obj +=  static_cast<TOPTW::Client*>(this->clients[j - 1])->getProfit() * arcs[i][j];
        }
    }
    model.add(IloMaximize(model.getEnv(), obj));
}

void TOPTW::Problem::addRoutingConstraints()
{
    IloExpr expr(model.getEnv());
    for (unsigned i = 1; i <= clients.size(); ++i){
        expr += arcs[0][i];
    }
    model.add(expr <= IloInt(vehicles.size()));
    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(model.getEnv());
        for (unsigned j = 0; j <= clients.size(); ++j) {
            expr += arcs[i][j];
        }
        model.add(expr <= 1);
    }

    for (unsigned i = 1; i <= clients.size(); ++i) {
        IloExpr expr(model.getEnv());
        for (unsigned j = 0; j <= clients.size(); ++j) {
            expr += arcs[i][j] - arcs[j][i];
        }
        model.add(expr == 0);
    }

}
