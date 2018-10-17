#pragma once
#include "../../routines/callbacks.hpp"
#include "../../data/problem.hpp"
    #include "../../data/attributes.hpp"
#include "../../data/reader.hpp"
#include "../cvrptw/cvrptw.hpp"
#include <sstream>


namespace VRPTWSyn
{
    struct Synchronization
    {
            Synchronization(const std::vector<routing::Model*> & p_list_synchronization):list_synchronization(p_list_synchronization){}
            std::vector<routing::Model*> list_synchronization;
            std::vector<routing::Model*> getSynchronizations() const { return this->list_synchronization;}
            void addSynchronization(routing::Model * service){this->list_synchronization.push_back(service);}
    };

    struct Client
            :
            public CVRPTW::Client,
            public Synchronization
    {
            Client(unsigned id,
                   const routing::Demand &demand,
                   const routing::Duration &service,
                   const routing::TW &timewindow,
                   const routing::Duration &x,
                   const routing::Duration &y)
                :
                  CVRPTW::Client(id, demand, service, timewindow, x, y),
                  Synchronization(std::vector<routing::Model*>())

            {
                this->setID(id);
            }

    };
    struct Depot
            :
            public CVRPTW::Depot
    {
            Depot(unsigned id,
                  const routing::Duration &x,
                  const routing::Duration &y,
                  const routing::TW &timewindow)
                :
                  CVRPTW::Depot(id, x, y, timewindow)
            {
            }
    };
    struct Vehicle
            :
            public CVRPTW::Vehicle
    {
            Vehicle(unsigned id, routing::Demand capacity = std::numeric_limits<routing::Demand>::max())
                :  CVRPTW::Vehicle(id, capacity)
            {
            }

    };

    class Reader;
    class IncumbentCallback;
    class Problem
            : public CVRPTW::Problem
    {
        public :
            friend class Reader;
            friend class IncumbentCallback;
            virtual IloModel generateModel(IloEnv &env) override ;
            virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;
            virtual routing::callback::IncumbentCallback *setIncumbentCallback(IloEnv &env) override;


        protected :
            virtual void addVariables() override ;
            virtual void addSequenceConstraints() override ;
            virtual void addTotalDistanceObjective() override ;
            virtual void addSynchronizationConstraints();
            virtual void addnvConstraint();
            IloExpr nv;
    };
    class Reader
            : public routing::Reader
    {
        public:
            virtual VRPTWSyn::Problem *readFile(std::string filepath);
    };

    struct Solution : public CVRP::Solution{
        public:

            virtual double getCost() override{
                return CVRP::Solution::getCost()
                        * 9.0 / static_cast<Depot*>(static_cast<Problem*>(problem)->depots[0])->getTwClose();
            }

    };

    class HeuristicCallback
            : public CVRPTW::HeuristicCallback
    {
        public :
            HeuristicCallback(IloEnv env, Problem *_problem,
                              routing::Constructor * p_constructor,
                              routing::Destructor * p_destructor)
                :  CVRPTW::HeuristicCallback(env, _problem, p_constructor, p_destructor)
            {
            }

    };

    class IncumbentCallback
            : public CVRP::IncumbentCallback
    {
        public :
            IncumbentCallback(IloEnv env, Problem *_problem)
                :CVRP::IncumbentCallback(env, _problem)
            {
            }


        protected:
            virtual void extractIncumbentSolution() override{
                CVRP::IncumbentCallback::extractIncumbentSolution();
                solution = new Solution(*static_cast<Solution*>(CVRP::IncumbentCallback::solution));
            }

    };
}
