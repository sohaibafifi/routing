#pragma once

#include "core/interfaces/ICPBackend.hpp"
#include <map>
#include <sstream>
#include <variant>

namespace routing {
namespace cp {

struct XCSP3Variable {
    std::string name;
    int lb;
    int ub;
    bool isBool;
};

struct XCSP3Interval {
    int startVarId;
    int endVarId;
    int sizeVarId;
    int presenceVarId; // -1 if not optional
};

class XCSP3Backend : public ICPBackend {
public:
    XCSP3Backend();
    ~XCSP3Backend() override = default;

    std::string name() const override { return "xcsp3"; }

    // Variable Creation
    IntVar newIntVar(int lb, int ub, const std::string& name = "") override;
    IntVar newBoolVar(const std::string& name = "") override;
    IntVar newConstant(int value) override;
    IntervalVar newIntervalVar(int minStart, int maxEnd, int minDuration, int maxDuration, const std::string& name = "") override;
    OptionalIntervalVar newOptionalIntervalVar(int minStart, int maxEnd, int minDuration, int maxDuration, const std::string& name = "") override;

    // Basic Constraints
    void addEquality(const LinearExpr& expr, int value) override;
    void addEquality(IntVar var1, IntVar var2) override;
    void addLinearConstraint(const LinearExpr& expr, int lb, int ub) override;
    void addLessOrEqual(IntVar var1, IntVar var2, int offset = 0) override;
    void addImplication(IntVar condition, const LinearExpr& expr, int lb, int ub) override;

    // Global Constraints
    void addAllDifferent(const std::vector<IntVar>& vars) override;
    void addElement(IntVar index, const std::vector<int>& array, IntVar result) override;
    void addElement(IntVar index, const std::vector<IntVar>& vars, IntVar result) override;
    void addCircuit(const std::vector<IntVar>& next) override;
    void addSubCircuit(const std::vector<IntVar>& next) override;
    void addInverse(const std::vector<IntVar>& next, const std::vector<IntVar>& prev) override;

    // Scheduling Constraints
    void addNoOverlap(const std::vector<IntervalVar>& intervals) override;
    void addNoOverlap(const std::vector<OptionalIntervalVar>& intervals) override;
    void addCumulative(const std::vector<IntervalVar>& intervals, const std::vector<int>& demands, int capacity) override;
    void addCumulative(const std::vector<OptionalIntervalVar>& intervals, const std::vector<int>& demands, int capacity) override;
    void addEndBeforeStart(IntervalVar interval1, IntervalVar interval2, int delay = 0) override;

    // Interval Accessors
    IntVar startOf(IntervalVar interval) override;
    IntVar endOf(IntervalVar interval) override;
    IntVar durationOf(IntervalVar interval) override;
    IntVar startOf(OptionalIntervalVar interval, int defaultValue = 0) override;
    IntVar endOf(OptionalIntervalVar interval, int defaultValue = 0) override;

    // Objective
    void minimize(const LinearExpr& expr) override;
    void maximize(const LinearExpr& expr) override;
    void minimizeMakespan(const std::vector<IntervalVar>& intervals) override;

    // Solving
    CPStatus solve(double timeout = 3600.0) override;
    void setHint(const std::vector<std::pair<IntVar, int>>& hints) override;
    void setNumWorkers(int workers) override;
    void setRandomSeed(int seed) override;

    // Solution Access
    int getValue(IntVar var) const override;
    bool getBoolValue(IntVar var) const override;
    int getStart(IntervalVar interval) const override;
    int getEnd(IntervalVar interval) const override;
    int getDuration(IntervalVar interval) const override;
    bool isPresent(OptionalIntervalVar interval) const override;
    double getObjectiveValue() const override;
    double getObjectiveBound() const override;
    double getSolveTime() const override;
    long long getNumBranches() const override;
    long long getNumFailures() const override;

    void clear() override;
    bool exportModel(const std::string& filename) const override;

private:
    std::vector<XCSP3Variable> variables_;
    std::vector<XCSP3Interval> intervals_;
    std::vector<XCSP3Interval> optionalIntervals_;
    std::vector<std::string> constraints_; // XML chunks
    std::string objectiveXml_;
    bool isMinimization_ = true;

    // Solution state
    bool solved_ = false;
    std::map<int, int> integerValues_;
    double bestObjective_ = 0.0;
    double solveTime_ = 0.0;
    
    // Helper to generate unique names
    std::string getVarName(int id) const;
};

} // namespace cp
} // namespace routing
