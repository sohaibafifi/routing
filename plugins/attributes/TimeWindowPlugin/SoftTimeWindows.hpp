// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/interfaces/IAttribute.hpp"
#include <memory>

namespace routing {
namespace attributes {

    /**
     * @brief Soft time window configuration attribute
     *
     * Used for CVRPSTW (CVRP with Soft Time Windows) problems.
     * Unlike hard time windows, soft time windows allow violations
     * with penalties for early arrival (wait) and late arrival (delay).
     *
     * This attribute is typically attached to the depot to configure
     * problem-wide penalty settings.
     */
    struct SoftTimeWindows : public Attribute<SoftTimeWindows> {
        SoftTimeWindows(double waitPenalty = 1.0, double delayPenalty = 1.0)
            : waitPenalty_(waitPenalty), delayPenalty_(delayPenalty) {}

        double getWaitPenalty() const { return waitPenalty_; }
        void setWaitPenalty(double penalty) { waitPenalty_ = penalty; }

        double getDelayPenalty() const { return delayPenalty_; }
        void setDelayPenalty(double penalty) { delayPenalty_ = penalty; }

        // IAttribute implementation
        std::unique_ptr<IAttribute> clone() const override {
            return std::make_unique<SoftTimeWindows>(waitPenalty_, delayPenalty_);
        }

        std::string name() const override {
            return "SoftTimeWindows";
        }

    private:
        double waitPenalty_;
        double delayPenalty_;
    };

} // namespace attributes
} // namespace routing
