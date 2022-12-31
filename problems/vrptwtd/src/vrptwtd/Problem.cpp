// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <utilities/Utilities.hpp>
#include "Problem.hpp"
#include "attributes/Synced.hpp"

#ifdef CPLEX_FOUND

void vrptwtd::Problem::addSynchronisationConstraints() {

    for (unsigned i = 1; i <= clients.size(); ++i) {
        if (auto *client = dynamic_cast<vrptwtd::attributes::Synced *>(clients[i - 1])){
            for (unsigned j = 0; j < client->getBrothersCount(); ++j) {
                auto brother = dynamic_cast<routing::Model * >(client->getBrother(j));
                IloConstraint constraint = start[client->getID()]  + client->getDelta(j) <=  start[brother->getID()];

                constraint.setName(std::string("sync_" + Utilities::itos(client->getID()) + "_" + Utilities::itos(brother->getID())).c_str());
                model.add(constraint);

                model.add(arcs[client->getID()][brother->getID()] == 0);
            }

        }
    }

}

routing::callback::HeuristicCallback *vrptwtd::Problem::setHeuristicCallback() {
    return nullptr;
}

routing::callback::UserCutCallback *vrptwtd::Problem::setUserCutCallback() {
    return nullptr;
}

routing::callback::LazyConstraintCallback *vrptwtd::Problem::setLazyConstraintCallback() {
    return nullptr;
}

routing::callback::InformationCallback *vrptwtd::Problem::setInformationCallback() {
    return nullptr;
}

routing::callback::IncumbentCallback *vrptwtd::Problem::setIncumbentCallback() {
    return nullptr;
}
#endif