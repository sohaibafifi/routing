// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "IEvaluator.hpp"
#include "core/cache/RouteCache.hpp"

namespace routing {

/**
 * @brief Context for removal operations
 */
struct RemovalContext {
    int position;           ///< Position to remove from (0-indexed)
    Entity* client;         ///< Client being removed
    Entity* predecessor;    ///< Node before removed client (may be depot)
    Entity* successor;      ///< Node after removed client (may be depot)
};

/**
 * @brief Context for 2-opt operations (segment reversal)
 *
 * After 2-opt on segment [i, j]:
 *   Original: ... -> node[i] -> node[i+1] -> ... -> node[j] -> node[j+1] -> ...
 *   Result:   ... -> node[i] -> node[j]   -> ... -> node[i+1] -> node[j+1] -> ...
 *
 * The segment [i+1, j] is reversed.
 */
struct TwoOptContext {
    int i;  ///< Position before reversed segment (inclusive boundary)
    int j;  ///< End of reversed segment (inclusive)
};

/**
 * @brief Result of incremental move evaluation
 */
struct MoveDelta {
    double costDelta = 0.0;  ///< Change in cost (positive = worse, negative = better)
    bool feasible = true;     ///< Whether move maintains feasibility

    MoveDelta() = default;
    MoveDelta(double delta, bool feas) : costDelta(delta), feasible(feas) {}

    /// Create an infeasible result
    static MoveDelta infeasible() { return MoveDelta(0.0, false); }

    /// Check if this is an improving feasible move
    bool isImproving() const { return feasible && costDelta < -1e-9; }
};

/**
 * @brief Extended evaluator interface supporting O(1) incremental evaluation
 *
 * This interface extends IEvaluator with methods that use cached route state
 * to evaluate moves in constant time instead of O(n).
 *
 * ## Cache-Based Evaluation
 *
 * Instead of re-simulating the entire route for each candidate move,
 * incremental evaluators use precomputed values stored in RouteCache:
 *
 * - **Time Windows**: Use maxForwardShift to check if delay propagates feasibly
 * - **Capacity**: Use totalLoad to check if new demand fits
 * - **Distance**: Use prefix sums for segment cost calculations
 *
 * ## Usage Pattern
 *
 * ```cpp
 * // In neighborhood:
 * tour->ensureCache();
 * const RouteCache& cache = tour->getCache();
 *
 * for each candidate move:
 *     MoveDelta delta = evaluator->evaluateInsertionIncremental(tour, cache, ctx);
 *     if (delta.feasible && delta.isImproving()) {
 *         // Accept move
 *     }
 *
 * // After applying move:
 * tour->invalidateCache();
 * ```
 *
 * ## Backward Compatibility
 *
 * This interface extends IEvaluator. Implementations should also implement
 * the base checkFeasibility() and evaluateInsertionDelta() methods for
 * code that doesn't use caching.
 */
class IIncrementalEvaluator : public IEvaluator {
public:
    virtual ~IIncrementalEvaluator() = default;

    // ========== Cache Management ==========

    /**
     * @brief Build/rebuild the cache for a tour
     *
     * Called when cache is invalid or after structural changes.
     * This is an O(n) operation that populates all cached values
     * for subsequent O(1) evaluations.
     *
     * @param tour The tour to cache
     * @param cache The cache to populate
     */
    virtual void buildCache(const Tour& tour, RouteCache& cache) const = 0;

    /**
     * @brief Update cache after an insertion
     *
     * Called after a client is inserted into the tour.
     * May be O(1) for some evaluators (capacity) or O(n) for others (time windows).
     * Default implementation rebuilds entire cache.
     *
     * @param tour The modified tour
     * @param cache The cache to update
     * @param ctx The insertion that was applied
     */
    virtual void updateCacheAfterInsertion(
        const Tour& tour,
        RouteCache& cache,
        const InsertionContext& ctx) const {
        // Default: full rebuild
        buildCache(tour, cache);
    }

    /**
     * @brief Update cache after a removal
     *
     * Called after a client is removed from the tour.
     * Default implementation rebuilds entire cache.
     *
     * @param tour The modified tour
     * @param cache The cache to update
     * @param ctx The removal that was applied
     */
    virtual void updateCacheAfterRemoval(
        const Tour& tour,
        RouteCache& cache,
        const RemovalContext& ctx) const {
        // Default: full rebuild
        buildCache(tour, cache);
    }

    // ========== Incremental Evaluation ==========

    /**
     * @brief O(1) insertion feasibility check using cache
     *
     * @param tour The tour to insert into
     * @param cache The precomputed cache for this tour
     * @param ctx The insertion to evaluate
     * @return true if insertion maintains feasibility
     */
    virtual bool checkFeasibilityIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const InsertionContext& ctx) const = 0;

    /**
     * @brief O(1) insertion evaluation using cache
     *
     * Returns both feasibility and cost delta.
     *
     * @param tour The tour to insert into
     * @param cache The precomputed cache for this tour
     * @param ctx The insertion to evaluate
     * @return MoveDelta with cost change and feasibility
     */
    virtual MoveDelta evaluateInsertionIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const InsertionContext& ctx) const = 0;

    /**
     * @brief O(1) removal evaluation using cache
     *
     * @param tour The tour to remove from
     * @param cache The precomputed cache for this tour
     * @param ctx The removal to evaluate
     * @return MoveDelta with cost change and feasibility
     */
    virtual MoveDelta evaluateRemovalIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const RemovalContext& ctx) const = 0;

    /**
     * @brief 2-opt move evaluation using cache
     *
     * Evaluates reversing segment [i+1, j] in the tour.
     * Complexity depends on evaluator:
     * - Distance: O(1) using edge costs
     * - Capacity: O(1) - reversal doesn't change total load
     * - Time Windows: O(j-i) - must simulate reversed segment
     *
     * @param tour The tour to modify
     * @param cache The precomputed cache for this tour
     * @param ctx The 2-opt move to evaluate
     * @return MoveDelta with cost change and feasibility
     */
    virtual MoveDelta evaluateTwoOptIncremental(
        const Tour& tour,
        const RouteCache& cache,
        const TwoOptContext& ctx) const = 0;
};

} // namespace routing
