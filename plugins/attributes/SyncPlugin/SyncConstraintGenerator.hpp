// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IConstraintGenerator.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/SyncPlugin/Synced.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"

namespace routing {
namespace constraints {

    /**
     * @brief Synchronization constraint generator for VRPTWTD
     *
     * Implements temporal synchronization constraints:
     * - If client A has a sync relation to client B with delta D:
     *   start[A] + D <= start[B]
     * - Additionally, direct arcs between synced clients are forbidden
     *
     * This is used for problems where services at different nodes
     * must be coordinated in time (e.g., home healthcare, technician routing).
     *
     * Requires: Synced attribute (and implicitly Rendezvous for start times)
     */
    class SyncConstraintGenerator : public IConstraintGenerator {
    public:
        std::string name() const override {
            return "SyncConstraintGenerator";
        }

        std::vector<AttributeTypeId> requiredAttributes() const override {
            return {
                std::type_index(typeid(attributes::Synced))
            };
        }

        int priority() const override {
            return 70;  // After time windows (60)
        }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem& problem) override {
            // No additional variables needed - uses start time variables
            // from TimeWindowConstraintGenerator
        }

        void addConstraints(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            size_t n = clients.size();

            // Build ID to index mapping for looking up brother indices
            std::map<unsigned, size_t> idToIndex;
            for (size_t i = 0; i < n; ++i) {
                idToIndex[clients[i]->getID()] = i + 1;  // +1 because index 0 is depot
            }

            // Add synchronization constraints for each synced client
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                auto* synced = client->tryGetAttribute<attributes::Synced>();

                if (!synced) continue;

                size_t clientIdx = i + 1;  // Client index (1-based, 0 is depot)

                for (size_t j = 0; j < synced->getBrothersCount(); ++j) {
                    unsigned brotherId = synced->getBrotherId(j);
                    Duration delta = synced->getDelta(j);

                    // Find brother's index
                    auto it = idToIndex.find(brotherId);
                    if (it == idToIndex.end()) continue;

                    size_t brotherIdx = it->second;

                    // Synchronization constraint: start[client] + delta <= start[brother]
                    // This uses the start time variables from TimeWindowConstraintGenerator
                    // We need to access them through the problem's start variable array
                    if (clientIdx < problem.startTime.size() &&
                        brotherIdx < problem.startTime.size()) {

                        std::string constraintName = "sync_" + std::to_string(client->getID())
                                                   + "_" + std::to_string(brotherId);

                        IloConstraint constraint =
                            problem.startTime[clientIdx] + delta <= problem.startTime[brotherIdx];
                        constraint.setName(constraintName.c_str());
                        problem.model.add(constraint);
                    }

                    // Forbid direct arc between synced clients
                    // (they must be visited by different vehicles or with other nodes in between)
                    if (clientIdx <= n && brotherIdx <= n) {
                        problem.model.add(problem.arcs[clientIdx][brotherIdx] == 0);
                    }
                }
            }
        }
#endif
    };

} // namespace constraints
} // namespace routing
