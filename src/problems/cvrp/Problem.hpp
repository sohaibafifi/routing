#pragma once
#include <sstream>

#include "../../routines/callbacks.hpp"
#include "../../data/problem.hpp"
#include "../../data/attributes.hpp"
#include "../../data/reader.hpp"
#include "models/Depot.hpp"
#include "models/Client.hpp"
#include "models/Vehicle.hpp"

namespace CVRP
{
    class Reader;
    class HeuristicCallback;
    class IncumbentCallback;
    class Problem
            : public routing::Problem
    {
            friend class HeuristicCallback;
            friend class IncumbentCallback;

            friend class Reader;
        public :
            virtual IloModel generateModel(IloEnv &env) override;
            std::vector<IloNumVar> consumption;

            virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) override;
            virtual routing::callback::IncumbentCallback *setIncumbentCallback(IloEnv &env) override;
            virtual Depot * getDepot();
            virtual void setDepot(Depot * depot);
        protected :
            virtual void addVariables() override;
            virtual void addSequenceConstraints() override;
            virtual void addCapacityConstraints();
            virtual void addObjective() override;
            virtual void addTotalDistanceObjective();

    };
}
