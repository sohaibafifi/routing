// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace routing {

// Forward declaration
class Solution;

/**
 * @brief Represents a move in a neighborhood
 */
struct NeighborhoodMove {
    std::string type;           // Move type identifier
    double delta = 0.0;         // Cost change (negative = improvement)
    std::vector<int> data;      // Move-specific data

    bool isImproving() const { return delta < 0.0; }
};

/**
 * @brief Interface for neighborhood operators
 */
class INeighborhood {
public:
    virtual ~INeighborhood() = default;

    /// Get neighborhood name
    virtual std::string name() const = 0;

    /// Get neighborhood description
    virtual std::string description() const { return ""; }

    /// Explore the neighborhood and find the best move
    virtual std::optional<NeighborhoodMove> explore(Solution& solution) = 0;

    /// Apply a move to the solution
    virtual void apply(Solution& solution, const NeighborhoodMove& move) = 0;

    /// Convenience method: explore and apply best move
    virtual bool improve(Solution& solution) {
        auto move = explore(solution);
        if (move && move->isImproving()) {
            apply(solution, *move);
            return true;
        }
        return false;
    }
};

} // namespace routing
