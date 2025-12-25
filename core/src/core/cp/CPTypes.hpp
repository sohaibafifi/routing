// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <vector>
#include <limits>

namespace routing {
namespace cp {

/**
 * @brief Handle for an integer variable in CP model
 *
 * This is a lightweight handle that references a variable in a backend.
 * The actual variable is owned by the backend implementation.
 */
class IntVar {
    int id_ = -1;
    friend class ICPBackend;

public:
    IntVar() = default;
    explicit IntVar(int id) : id_(id) {}

    bool isValid() const { return id_ >= 0; }
    int id() const { return id_; }

    bool operator==(const IntVar& other) const { return id_ == other.id_; }
    bool operator!=(const IntVar& other) const { return id_ != other.id_; }
};

/**
 * @brief Handle for an interval variable in CP model
 *
 * Represents a time interval with start, end, and duration.
 * Used for scheduling constraints (no-overlap, cumulative, etc.)
 */
class IntervalVar {
    int id_ = -1;
    friend class ICPBackend;

public:
    IntervalVar() = default;
    explicit IntervalVar(int id) : id_(id) {}

    bool isValid() const { return id_ >= 0; }
    int id() const { return id_; }

    bool operator==(const IntervalVar& other) const { return id_ == other.id_; }
    bool operator!=(const IntervalVar& other) const { return id_ != other.id_; }
};

/**
 * @brief Handle for an optional interval variable
 *
 * Like IntervalVar but can be absent (not scheduled).
 * Used for optional activities.
 */
class OptionalIntervalVar {
    int id_ = -1;
    IntVar presence_;
    friend class ICPBackend;

public:
    OptionalIntervalVar() = default;
    OptionalIntervalVar(int id, IntVar presence) : id_(id), presence_(presence) {}

    bool isValid() const { return id_ >= 0; }
    int id() const { return id_; }
    IntVar presenceVar() const { return presence_; }

    bool operator==(const OptionalIntervalVar& other) const { return id_ == other.id_; }
    bool operator!=(const OptionalIntervalVar& other) const { return id_ != other.id_; }
};

/**
 * @brief Linear expression over integer variables
 *
 * Represents: constant + sum(coeff[i] * var[i])
 */
class LinearExpr {
    std::vector<std::pair<IntVar, int>> terms_;
    int constant_ = 0;

public:
    LinearExpr() = default;
    explicit LinearExpr(int constant) : constant_(constant) {}
    explicit LinearExpr(IntVar var) { terms_.emplace_back(var, 1); }
    LinearExpr(IntVar var, int coeff) { terms_.emplace_back(var, coeff); }

    LinearExpr& addTerm(IntVar var, int coeff = 1) {
        if (coeff != 0) {
            terms_.emplace_back(var, coeff);
        }
        return *this;
    }

    LinearExpr& addConstant(int c) {
        constant_ += c;
        return *this;
    }

    LinearExpr& operator+=(const LinearExpr& other) {
        for (const auto& term : other.terms_) {
            terms_.push_back(term);
        }
        constant_ += other.constant_;
        return *this;
    }

    LinearExpr& operator+=(IntVar var) {
        terms_.emplace_back(var, 1);
        return *this;
    }

    LinearExpr& operator-=(IntVar var) {
        terms_.emplace_back(var, -1);
        return *this;
    }

    LinearExpr& operator*=(int coeff) {
        for (auto& term : terms_) {
            term.second *= coeff;
        }
        constant_ *= coeff;
        return *this;
    }

    const std::vector<std::pair<IntVar, int>>& terms() const { return terms_; }
    int constant() const { return constant_; }

    bool isEmpty() const { return terms_.empty() && constant_ == 0; }
};

// Convenience operators
inline LinearExpr operator+(IntVar a, IntVar b) {
    LinearExpr expr;
    expr.addTerm(a, 1);
    expr.addTerm(b, 1);
    return expr;
}

inline LinearExpr operator+(IntVar var, int constant) {
    LinearExpr expr(constant);
    expr.addTerm(var, 1);
    return expr;
}

inline LinearExpr operator-(IntVar a, IntVar b) {
    LinearExpr expr;
    expr.addTerm(a, 1);
    expr.addTerm(b, -1);
    return expr;
}

inline LinearExpr operator*(IntVar var, int coeff) {
    return LinearExpr(var, coeff);
}

inline LinearExpr operator*(int coeff, IntVar var) {
    return LinearExpr(var, coeff);
}

/**
 * @brief Solver status after solving
 */
enum class CPStatus {
    Unknown,           /// Not yet solved or no status
    Optimal,           /// Proven optimal solution found
    Feasible,          /// Feasible solution found (not proven optimal)
    Infeasible,        /// Problem is infeasible
    Unbounded,         /// Problem is unbounded
    ModelInvalid,      /// Model is invalid
    Error              /// Solver error
};

inline const char* toString(CPStatus status) {
    switch (status) {
        case CPStatus::Unknown: return "Unknown";
        case CPStatus::Optimal: return "Optimal";
        case CPStatus::Feasible: return "Feasible";
        case CPStatus::Infeasible: return "Infeasible";
        case CPStatus::Unbounded: return "Unbounded";
        case CPStatus::ModelInvalid: return "ModelInvalid";
        case CPStatus::Error: return "Error";
    }
    return "Unknown";
}

} // namespace cp
} // namespace routing
