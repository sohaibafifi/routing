#pragma once

#include "attributes.hpp"

#include "core/data/models/Client.hpp"
#include "core/data/models/Depot.hpp"
#include "core/data/models/Vehicle.hpp"
#include <memory>
#include <string>
#include <vector>
#include "core/data/attributes/GeoNode.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

#include <ilcplex/ilocplexi.h>

#pragma GCC diagnostic pop

namespace routing {
    namespace callback {
        class UserCutCallback;

        class HeuristicCallback;

        class IncumbentCallback;

        class LazyConstraintCallback;

        class InformationCallback;
    }
    class Problem {
    public :
        virtual ~Problem() {
            for (unsigned i = 0; i < clients.size(); ++i) delete clients[i];
            for (unsigned k = 0; k < vehicles.size(); ++k) delete vehicles[k];
            for (unsigned d = 0; d < depots.size(); ++d) delete depots[d];
        }

        template<class Reader>
        static Problem *loadFromFile(std::string filepath) {
            return Reader().readFile(filepath);
        }

        virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) = 0;

        virtual routing::callback::IncumbentCallback *setIncumbentCallback(IloEnv &env) = 0;

        virtual routing::callback::UserCutCallback *setUserCutCallback(IloEnv &env) = 0;

        virtual routing::callback::LazyConstraintCallback *setLazyConstraintCallback(IloEnv &env) = 0;

        virtual routing::callback::InformationCallback *setInformationCallback(IloEnv &env) = 0;

        std::string getName() const {
            return name;
        }

        void setName(const std::string &value) {
            name = value;
        }

        virtual routing::Duration getDistance(const models::Client &c1, const models::Client &c2) const = 0;

        virtual routing::Duration getDistance(const models::Client &c1, const models::Depot &d) const = 0;

        IloModel model;

        std::vector<models::Vehicle *> vehicles;
        std::vector<models::Depot *> depots;
        std::vector<models::Client *> clients;
        IloExpr obj;

        virtual IloModel generateModel(IloEnv &env) {
            this->model = IloModel(env);
            this->addVariables();
            this->addConstraints();

            this->addObjective();
            return this->model;
        }

        std::string name;

    protected:
        virtual void addVariables() = 0;

        virtual void addConstraints() = 0;


        virtual void addObjective() = 0;
    };
}
