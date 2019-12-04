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
#ifdef CPLEX
#include <ilcplex/ilocplexi.h>
#endif
#pragma GCC diagnostic pop

namespace routing {
#ifdef CPLEX
    namespace callback {
        class UserCutCallback;
        class HeuristicCallback;
        class IncumbentCallback;
        class LazyConstraintCallback;
        class InformationCallback;
    }
#endif

    namespace models {
        class Solution;

        class Tour;
    }
    class Problem;

    class Initializer {
        Problem *problem;
    public:
        Initializer(Problem *p_problem)
                : problem(p_problem) {
        }

        Problem *getProblem() const { return problem; }
        virtual models::Solution *initialSolution() = 0;
        virtual models::Tour *initialTour(int vehicleID) = 0;
    };

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

        virtual Initializer *initializer() = 0;

#ifdef CPLEX
        virtual routing::callback::HeuristicCallback *setHeuristicCallback(IloEnv &env) { return nullptr; }

        virtual routing::callback::IncumbentCallback *setIncumbentCallback(IloEnv &env) { return nullptr; }

        virtual routing::callback::UserCutCallback *setUserCutCallback(IloEnv &env) { return nullptr; }

        virtual routing::callback::LazyConstraintCallback *setLazyConstraintCallback(IloEnv &env) { return nullptr; }

        virtual routing::callback::InformationCallback *setInformationCallback(IloEnv &env) { return nullptr; }
#endif
        std::string getName() const {
            return name;
        }

        void setName(const std::string &value) {
            name = value;
        }

        virtual routing::Duration getDistance(const models::Client &c1, const models::Client &c2) const = 0;

        virtual routing::Duration getDistance(const models::Client &c1, const models::Depot &d) const = 0;

#ifdef CPLEX

        IloModel model;
        IloExpr obj;
        virtual IloModel generateModel(IloEnv &env) {
            this->model = IloModel(env);
            this->model.setName(this->getName().c_str());
            this->addVariables();
            this->addConstraints();
            this->addObjective();
            return this->model;
        }
#endif

        std::vector<models::Vehicle *> vehicles;
        std::vector<models::Depot *> depots;
        std::vector<models::Client *> clients;

        std::string name;

    protected:
#ifdef CPLEX

        virtual void addVariables() = 0;

        virtual void addConstraints() = 0;

        virtual void addObjective() = 0;
#endif

    };
}
