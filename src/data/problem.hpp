#pragma once

#include "attributes.hpp"

#include "models/Client.hpp"
#include "models/Depot.hpp"
#include "models/Vehicle.hpp"
#include <memory>
#include <string>
#include <vector>
#include "attributes/GeoNode.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcplex/ilocplex.h>

#pragma GCC diagnostic pop

namespace routing
{
    namespace callback
    {
        class UserCutCallback;

        class HeuristicCallback;
        class IncumbentCallback;
        class LazyConstraintCallback;
        class InformationCallback;
    }
    class Problem
    {
        public :
            virtual ~Problem();
            template<class Reader>
            static Problem *loadFromFile(std::string filepath)
            {
                return Reader().readFile(filepath);
            }

            virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env);
            virtual routing::callback::IncumbentCallback *setIncumbentCallback(IloEnv &env);
            virtual routing::callback::UserCutCallback *setUserCutCallback(IloEnv &env);
            virtual routing::callback::LazyConstraintCallback *setLazyConstraintCallback(IloEnv &env);
            virtual routing::callback::InformationCallback *setInformationCallback(IloEnv &env);

            std::string getName() const;
            void setName(const std::string &value);

            virtual routing::Duration getDistance(const models::Client &c1, const models::Client &c2) const;
            virtual routing::Duration getDistance(const models::Client &c1, const models::Depot &d) const;

            IloModel model;
            std::vector<std::vector<IloNumVar> > arcs;
            std::vector<std::vector<IloNumVar> > affectation;
            std::vector<IloNumVar> order;
            std::vector<std::vector<routing::Duration> > distances, distances_to_depots;
            std::vector<models::Vehicle *> vehicles;
            std::vector<models::Depot *> depots;
            std::vector<models::Client *> clients;
            IloExpr obj;

            virtual IloModel generateModel(IloEnv &env) = 0;
        protected:
            virtual void addVariables();
            virtual void addAffectationConstraints();
            virtual void addRoutingConstraints();
            virtual void addSequenceConstraints();
            std::string name;

            virtual void addObjective() = 0;
    };
}
