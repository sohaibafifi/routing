#pragma once
#include "../../routines/callbacks.hpp"
#include "../../data/problem.hpp"
#include "../../data/attributes.hpp"
#include "../../data/reader.hpp"
#include "../cvrp/cvrp.hpp"
#include <sstream>
#include "../../data/attributes/Rendezvous.hpp"
#include "../../data/attributes/ServiceQuery.hpp"
#include "../../data/attributes/Service.hpp"
#include "../../data/attributes/Appointment.hpp"

// TODO :  implement the operators for vrptw
#include "../cvrp/routines/operators/Constructor.hpp"
#include "../cvrp/routines/operators/Destructor.hpp"
namespace CVRPTW
{
    struct Client
            :
            public CVRP::Client,
            public routing::attributes::Rendezvous,
            public routing::attributes::ServiceQuery
    {
            Client(unsigned id,
                   const routing::Demand &demand,
                   const routing::Duration &service,
                   const routing::TW &timewindow,
                   const routing::Duration &x,
                   const routing::Duration &y)
                :
                  CVRP::Client(id, demand, x, y),
                  routing::attributes::Rendezvous(timewindow),
                  routing::attributes::ServiceQuery(service)
            {
             }

    };
    struct Visit:
            public routing::attributes::Service,
            public routing::attributes::Appointment
    {
            Visit(Client * p_client,
                  const routing::Duration & p_start,
                  const routing::Duration& p_maxshift,
                  const routing::Duration& p_wait):
                routing::attributes::Service(p_start),
                routing::attributes::Appointment(p_maxshift, p_wait),
                client(p_client)
            {}
            Client * client;
    };
    struct Depot
            :
            public CVRP::Depot,
            public routing::attributes::Rendezvous
    {
            Depot(unsigned id,
                  const routing::Duration &x,
                  const routing::Duration &y,
                  const routing::TW &timewindow)
                :
                  CVRP::Depot(id, x, y),
                  routing::attributes::Rendezvous(timewindow)
            {
            }
    };
    struct Vehicle
            :
            public CVRP::Vehicle
    {
            Vehicle(unsigned id, routing::Capacity capacity)
                :  CVRP::Vehicle(id, capacity)
            {
            }
    };
    class Reader;
    class Problem
            : public CVRP::Problem
    {
        public :
            friend class Reader;
            virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;
        protected :
            virtual void addVariables() override ;
            virtual void addSequenceConstraints() override ;
            std::vector<IloNumVar> start;
    };

    struct Tour : public CVRP::Tour{

        public:
            Tour(Problem * p_problem, unsigned vehicleID):
                CVRP::Tour(p_problem, vehicleID){}
            virtual routing::Duration evaluateInsertion(routing::models::Client *client, unsigned position, bool &possible) override;
            virtual routing::Duration evaluateRemove(unsigned position) override;
            virtual void removeClient(unsigned position) override;
            virtual void addClient(routing::models::Client *client, unsigned position) override;

    };
    struct Solution : public CVRP::Solution{
        public:
            Solution(Problem * problem);

            virtual void updateMaxShifts();
            virtual void print(std::ostream &out) override;
            std::vector<Visit *> visits;
            virtual void update() override;
             virtual void addTour(routing::models::Tour *tour, unsigned position) override;
             virtual unsigned long getNbTour() const  override { return tours.size();}

             virtual routing::models::Solution *clone() const override;
            virtual routing::models::Tour *getTour(unsigned t) override;
        protected:
            std::vector<Tour*> tours;
    };

    class Reader
            : public routing::Reader
    {
        public:
            virtual CVRPTW::Problem *readFile(std::string filepath);
    };

    class HeuristicCallback
            : public CVRP::HeuristicCallback
    {
        public :
            HeuristicCallback(IloEnv env, Problem *_problem,
                              routing::Constructor * p_constructor,
                              routing::Destructor * p_destructor)
                :  CVRP::HeuristicCallback(env, _problem, p_constructor, p_destructor),
                  problem(_problem)
            {

            }
            Problem * problem; // avoid static_cast<Problem*>(problem)
            virtual void initSolution() override {
                solution = new Solution(problem);
            }
     };
}
