// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <vector>
#include <cstdint>
#include <limits>

namespace routing {

/**
 * @brief Cached state for a single route enabling O(1) move evaluation
 *
 * Stores precomputed values (arrival times, prefix sums, forward shift slack)
 * that allow incremental evaluators to check feasibility in constant time.
 *
 * The cache is invalidated after structural changes (insertions, removals)
 * and rebuilt lazily on next access.
 */
class RouteCache {
public:
    // ========== Time Window Cache ==========

    /// Earliest arrival time at each position (after travel from predecessor)
    std::vector<double> arrivalTime;

    /// Start of service at each position (max of arrival and TW open)
    std::vector<double> startTime;

    /// Departure time from each position (start + service duration)
    std::vector<double> departureTime;

    /// Waiting time at each position (start - arrival, i.e., idle time)
    std::vector<double> waitingTime;

    /**
     * @brief Maximum forward shift at each position
     *
     * maxForwardShift[i] = maximum amount by which departure from position i
     * can be delayed without violating any downstream time window constraint.
     *
     * This enables O(1) insertion feasibility checking:
     * - If the delay caused by insertion <= maxForwardShift[successor], feasible
     * - No need to simulate the entire downstream route
     *
     * Computed via backward pass:
     *   maxForwardShift[n-1] = (depotClose - returnTime) + waitingTime adjustment
     *   maxForwardShift[i] = min(close[i] - start[i], maxForwardShift[i+1] + waitingTime[i+1])
     */
    std::vector<double> maxForwardShift;

    /// Depot closing time (used for return-to-depot feasibility)
    double depotCloseTime = std::numeric_limits<double>::max();

    /// Return time to depot from last client
    double returnToDepotTime = 0.0;

    // ========== Capacity Cache ==========

    /// Cumulative load up to and including position i (prefix sum)
    std::vector<double> prefixLoad;

    /// Total route load (sum of all demands)
    double totalLoad = 0.0;

    /// Vehicle capacity for this route
    double vehicleCapacity = std::numeric_limits<double>::infinity();

    // ========== Distance Cache ==========

    /// Cumulative distance up to position i (prefix sum from depot)
    std::vector<double> prefixDistance;

    /// Total route distance (including return to depot)
    double totalDistance = 0.0;

    // ========== Cache Management ==========

    /// Cache version (incremented on each rebuild)
    uint64_t version = 0;

    /// Check if cache contains valid data
    bool isValid() const { return valid_; }

    /// Mark cache as invalid (requires rebuild)
    void invalidate() { valid_ = false; }

    /// Mark cache as valid after rebuild
    void setValid() { valid_ = true; }

    /// Resize all vectors for n clients (excluding depot)
    void resize(size_t n) {
        arrivalTime.resize(n);
        startTime.resize(n);
        departureTime.resize(n);
        waitingTime.resize(n);
        maxForwardShift.resize(n);
        prefixLoad.resize(n);
        prefixDistance.resize(n);
    }

    /// Clear all cached values and mark as invalid
    void clear() {
        arrivalTime.clear();
        startTime.clear();
        departureTime.clear();
        waitingTime.clear();
        maxForwardShift.clear();
        prefixLoad.clear();
        prefixDistance.clear();
        totalLoad = 0.0;
        totalDistance = 0.0;
        vehicleCapacity = std::numeric_limits<double>::infinity();
        depotCloseTime = std::numeric_limits<double>::max();
        returnToDepotTime = 0.0;
        valid_ = false;
    }

    /// Get number of clients in cached route
    size_t size() const { return arrivalTime.size(); }

private:
    bool valid_ = false;
};

} // namespace routing
