// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include <vector>
#include <memory>

namespace routing {
namespace attributes {

    /**
     * @brief Synchronization attribute for nodes requiring coordination
     *
     * Used in VRPTWTD (VRP with Time Windows and Temporal Dependencies)
     * and similar problems where service at one node must happen
     * before/after service at another node with a time delta.
     *
     * A synchronized node has "brothers" that must be visited with
     * a minimum time difference (delta).
     *
     * Example: In home healthcare, a nurse must visit patient A
     * before the doctor visits patient A (different service types).
     */
    struct Synced : public Attribute<Synced> {
        struct SyncRelation {
            unsigned brotherId;     // ID of the related node
            Duration delta;         // Minimum time after this node's service
        };

        Synced() = default;

        explicit Synced(const std::vector<SyncRelation>& relations)
            : relations_(relations) {}

        // Add a synchronization relationship
        void addBrother(unsigned brotherId, Duration delta) {
            relations_.push_back({brotherId, delta});
        }

        // Get number of sync relationships
        size_t getBrothersCount() const {
            return relations_.size();
        }

        // Get brother ID at index
        unsigned getBrotherId(size_t index) const {
            return relations_.at(index).brotherId;
        }

        // Get delta time at index
        Duration getDelta(size_t index) const {
            return relations_.at(index).delta;
        }

        // Get all relations
        const std::vector<SyncRelation>& getRelations() const {
            return relations_;
        }

        // IAttribute implementation
        std::unique_ptr<IAttribute> clone() const override {
            return std::make_unique<Synced>(relations_);
        }

        std::string name() const override {
            return "Synced";
        }

    private:
        std::vector<SyncRelation> relations_;
    };

} // namespace attributes
} // namespace routing
