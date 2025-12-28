// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <vector>
#include <limits>
#include <string>

namespace routing {
namespace mip {

/**
 * @brief Handle for a continuous variable in MIP model
 */
class Var {
    int id_ = -1;
    friend class IMIPBackend;

public:
    Var() = default;
    explicit Var(int id) : id_(id) {}

    bool isValid() const { return id_ >= 0; }
    int id() const { return id_; }

    bool operator==(const Var& other) const { return id_ == other.id_; }
    bool operator!=(const Var& other) const { return id_ != other.id_; }
};

/**
 * @brief Handle for an integer variable in MIP model
 */
class IntVar {
    int id_ = -1;
    friend class IMIPBackend;

public:
    IntVar() = default;
    explicit IntVar(int id) : id_(id) {}

    bool isValid() const { return id_ >= 0; }
    int id() const { return id_; }

    bool operator==(const IntVar& other) const { return id_ == other.id_; }
    bool operator!=(const IntVar& other) const { return id_ != other.id_; }

    // Allow implicit conversion to Var for use in expressions
    operator Var() const { return Var(id_); }
};

/**
 * @brief Handle for a binary variable in MIP model
 */
class BoolVar {
    int id_ = -1;
    friend class IMIPBackend;

public:
    BoolVar() = default;
    explicit BoolVar(int id) : id_(id) {}

    bool isValid() const { return id_ >= 0; }
    int id() const { return id_; }

    bool operator==(const BoolVar& other) const { return id_ == other.id_; }
    bool operator!=(const BoolVar& other) const { return id_ != other.id_; }

    // Allow implicit conversion for use in expressions
    operator Var() const { return Var(id_); }
    operator IntVar() const { return IntVar(id_); }
};

/**
 * @brief Linear expression over variables
 *
 * Represents: constant + sum(coeff[i] * var[i])
 */
class LinearExpr {
    std::vector<std::pair<int, double>> terms_;  // (var_id, coeff)
    double constant_ = 0.0;

public:
    LinearExpr() = default;
    explicit LinearExpr(double constant) : constant_(constant) {}
    explicit LinearExpr(Var var) { terms_.emplace_back(var.id(), 1.0); }
    LinearExpr(Var var, double coeff) { terms_.emplace_back(var.id(), coeff); }

    LinearExpr& addTerm(Var var, double coeff = 1.0) {
        if (coeff != 0.0) {
            terms_.emplace_back(var.id(), coeff);
        }
        return *this;
    }

    LinearExpr& addTerm(IntVar var, double coeff = 1.0) {
        return addTerm(Var(var.id()), coeff);
    }

    LinearExpr& addTerm(BoolVar var, double coeff = 1.0) {
        return addTerm(Var(var.id()), coeff);
    }

    LinearExpr& addConstant(double c) {
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

    LinearExpr& operator+=(Var var) {
        terms_.emplace_back(var.id(), 1.0);
        return *this;
    }

    LinearExpr& operator-=(Var var) {
        terms_.emplace_back(var.id(), -1.0);
        return *this;
    }

    LinearExpr& operator*=(double coeff) {
        for (auto& term : terms_) {
            term.second *= coeff;
        }
        constant_ *= coeff;
        return *this;
    }

    const std::vector<std::pair<int, double>>& terms() const { return terms_; }
    double constant() const { return constant_; }

    bool isEmpty() const { return terms_.empty() && constant_ == 0.0; }
};

// Convenience operators
inline LinearExpr operator+(Var a, Var b) {
    LinearExpr expr;
    expr.addTerm(a, 1.0);
    expr.addTerm(b, 1.0);
    return expr;
}

inline LinearExpr operator+(Var var, double constant) {
    LinearExpr expr(constant);
    expr.addTerm(var, 1.0);
    return expr;
}

inline LinearExpr operator+(double constant, Var var) {
    return var + constant;
}

inline LinearExpr operator-(Var a, Var b) {
    LinearExpr expr;
    expr.addTerm(a, 1.0);
    expr.addTerm(b, -1.0);
    return expr;
}

inline LinearExpr operator*(Var var, double coeff) {
    return LinearExpr(var, coeff);
}

inline LinearExpr operator*(double coeff, Var var) {
    return LinearExpr(var, coeff);
}

/**
 * @brief Quadratic expression over variables
 *
 * Represents: constant + sum(linear_coeff[i] * var[i]) + sum(quad_coeff[i,j] * var[i] * var[j])
 */
class QuadExpr {
    LinearExpr linear_;
    std::vector<std::tuple<int, int, double>> quadTerms_;  // (var_id1, var_id2, coeff)

public:
    QuadExpr() = default;
    explicit QuadExpr(const LinearExpr& linear) : linear_(linear) {}

    QuadExpr& addLinearTerm(Var var, double coeff = 1.0) {
        linear_.addTerm(var, coeff);
        return *this;
    }

    QuadExpr& addQuadTerm(Var var1, Var var2, double coeff = 1.0) {
        if (coeff != 0.0) {
            quadTerms_.emplace_back(var1.id(), var2.id(), coeff);
        }
        return *this;
    }

    const LinearExpr& linearPart() const { return linear_; }
    const std::vector<std::tuple<int, int, double>>& quadTerms() const { return quadTerms_; }
};

/**
 * @brief Constraint sense for linear constraints
 */
enum class Sense {
    LessEqual,     // <=
    Equal,         // ==
    GreaterEqual   // >=
};

/**
 * @brief Solver status after solving
 */
enum class MIPStatus {
    Unknown,           // Not yet solved or no status
    Optimal,           // Proven optimal solution found
    Feasible,          // Feasible solution found (not proven optimal)
    Infeasible,        // Problem is infeasible
    InfeasibleOrUnbounded,  // Problem is infeasible or unbounded
    Unbounded,         // Problem is unbounded
    NodeLimit,         // Node limit reached
    TimeLimit,         // Time limit reached
    SolutionLimit,     // Solution limit reached
    Error              // Solver error
};

inline const char* toString(MIPStatus status) {
    switch (status) {
        case MIPStatus::Unknown: return "Unknown";
        case MIPStatus::Optimal: return "Optimal";
        case MIPStatus::Feasible: return "Feasible";
        case MIPStatus::Infeasible: return "Infeasible";
        case MIPStatus::InfeasibleOrUnbounded: return "InfeasibleOrUnbounded";
        case MIPStatus::Unbounded: return "Unbounded";
        case MIPStatus::NodeLimit: return "NodeLimit";
        case MIPStatus::TimeLimit: return "TimeLimit";
        case MIPStatus::SolutionLimit: return "SolutionLimit";
        case MIPStatus::Error: return "Error";
    }
    return "Unknown";
}

/**
 * @brief Variable type enumeration
 */
enum class VarType {
    Continuous,
    Integer,
    Binary
};

/**
 * @brief Optimization sense
 */
enum class OptSense {
    Minimize,
    Maximize
};

} // namespace mip
} // namespace routing
