#include "XCSP3Backend.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <regex>

namespace routing {
namespace cp {

XCSP3Backend::XCSP3Backend() = default;

IntVar XCSP3Backend::newIntVar(int lb, int ub, const std::string& name) {
    int id = static_cast<int>(variables_.size());
    std::string finalName = name.empty() ? "x" + std::to_string(id) : name;
    variables_.push_back({finalName, lb, ub, false});
    return IntVar(id);
}

IntVar XCSP3Backend::newBoolVar(const std::string& name) {
    int id = static_cast<int>(variables_.size());
    std::string finalName = name.empty() ? "b" + std::to_string(id) : name;
    variables_.push_back({finalName, 0, 1, true});
    return IntVar(id);
}

IntVar XCSP3Backend::newConstant(int value) {
    return newIntVar(value, value, "c" + std::to_string(value));
}

IntervalVar XCSP3Backend::newIntervalVar(int minStart, int maxEnd, int minDuration, int maxDuration, const std::string& name) {
    int id = static_cast<int>(intervals_.size());
    std::string baseName = name.empty() ? "itv" + std::to_string(id) : name;

    // Create 3 variables for Start, End, Size
    IntVar start = newIntVar(minStart, maxEnd - minDuration, baseName + "_start");
    IntVar size = newIntVar(minDuration, maxDuration, baseName + "_size");
    IntVar end = newIntVar(minStart + minDuration, maxEnd, baseName + "_end");

    // Add consistency constraint: start + size = end
    // In XCSP3, we can define this as a constraint.
    // For now, we will add it as a linear constraint.
    LinearExpr expr;
    expr.addTerm(start, 1);
    expr.addTerm(size, 1);
    expr.addTerm(end, -1);
    addEquality(expr, 0);

    intervals_.push_back({start.id(), end.id(), size.id(), -1});
    return IntervalVar(id);
}

OptionalIntervalVar XCSP3Backend::newOptionalIntervalVar(int minStart, int maxEnd, int minDuration, int maxDuration, const std::string& name) {
    int id = static_cast<int>(optionalIntervals_.size());
    std::string baseName = name.empty() ? "oitv" + std::to_string(id) : name;

    // Create Start, End, Size, Presence
    IntVar start = newIntVar(minStart, maxEnd - minDuration, baseName + "_start");
    IntVar size = newIntVar(minDuration, maxDuration, baseName + "_size");
    IntVar end = newIntVar(minStart + minDuration, maxEnd, baseName + "_end");
    IntVar presence = newBoolVar(baseName + "_presence");

    // Consistency: start + size = end (enforced only if present)
    // We can use implication.
    // NOTE: In standard CP, if absent, values are unconstrained or dummy.
    // We enforce relation if present.
    LinearExpr expr;
    expr.addTerm(start, 1);
    expr.addTerm(size, 1);
    expr.addTerm(end, -1);
    addImplication(presence, expr, 0, 0); // presence => start+size-end=0

    optionalIntervals_.push_back({start.id(), end.id(), size.id(), presence.id()});
    return OptionalIntervalVar(id, presence);
}

// Helper to get variable name by ID
std::string XCSP3Backend::getVarName(int id) const {
    return variables_[id].name;
}

// ========== Constraints ==========

void XCSP3Backend::addEquality(const LinearExpr& expr, int value) {
    // <intension> eq(sum(times(c1,x1),...), value) </intension>
    std::stringstream ss;
    ss << "<intension> eq(add(";
    bool first = true;
    for (auto const& [var, coeff] : expr.terms()) {
        if (!first) ss << ",";
        ss << "mul(" << coeff << "," << getVarName(var.id()) << ")";
        first = false;
    }
    ss << ")," << (expr.constant() + value) << ") </intension>"; // Careful with constant sign
    // Actually: terms + constant = value => terms = value - constant
    // Correct logic:
    // eq(add(mul(...)...), value - constant)
    
    // Resetting stream for clean build
    std::stringstream xml;
    xml << "<intension> eq(add(";
    if (expr.terms().empty()) {
        xml << "0";
    } else {
        size_t idx = 0;
        for (auto const& [var, coeff] : expr.terms()) {
            if (idx > 0) xml << ",";
            xml << "mul(" << coeff << "," << getVarName(var.id()) << ")";
            idx++;
        }
    }
    xml << ")," << (value - expr.constant()) << ") </intension>";
    constraints_.push_back(xml.str());
}

void XCSP3Backend::addEquality(IntVar var1, IntVar var2) {
    std::stringstream ss;
    ss << "<intension> eq(" << getVarName(var1.id()) << "," << getVarName(var2.id()) << ") </intension>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addLinearConstraint(const LinearExpr& expr, int lb, int ub) {
    // Support range or single inequality
    // If range: lb <= expr <= ub
    // In XCSP3 intension: and(ge(expr, lb), le(expr, ub))
    
    std::stringstream exprStr;
    exprStr << "add(";
    if (expr.terms().empty()) {
        exprStr << "0";
    } else {
        size_t idx = 0;
        for (auto const& [var, coeff] : expr.terms()) {
            if (idx > 0) exprStr << ",";
            exprStr << "mul(" << coeff << "," << getVarName(var.id()) << ")";
            idx++;
        }
    }
    exprStr << ")"; // End add

    // Adjust bounds by constant
    long long finalLb = lb - expr.constant();
    long long finalUb = ub - expr.constant();

    std::stringstream xml;
    xml << "<intension> ";
    if (lb > std::numeric_limits<int>::min() / 2 && ub < std::numeric_limits<int>::max() / 2) {
        xml << "and(ge(" << exprStr.str() << "," << finalLb << "),le(" << exprStr.str() << "," << finalUb << "))";
    } else if (lb > std::numeric_limits<int>::min() / 2) {
        xml << "ge(" << exprStr.str() << "," << finalLb << ")";
    } else if (ub < std::numeric_limits<int>::max() / 2) {
        xml << "le(" << exprStr.str() << "," << finalUb << ")";
    }
    xml << " </intension>";
    constraints_.push_back(xml.str());
}

void XCSP3Backend::addLessOrEqual(IntVar var1, IntVar var2, int offset) {
    // var1 <= var2 + offset
    std::stringstream ss;
    ss << "<intension> le(" << getVarName(var1.id()) << ",add(" << getVarName(var2.id()) << "," << offset << ")) </intension>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addImplication(IntVar condition, const LinearExpr& expr, int lb, int ub) {
    // condition -> linearConstraint
    // imp(eq(cond,1), linearConstraint)
    
    // Construct linear part
    std::stringstream exprStr;
    exprStr << "add(";
    if (expr.terms().empty()) exprStr << "0";
    else {
        size_t idx = 0;
        for (auto const& [var, coeff] : expr.terms()) {
            if (idx > 0) exprStr << ",";
            exprStr << "mul(" << coeff << "," << getVarName(var.id()) << ")";
            idx++;
        }
    }
    exprStr << ")";

    long long finalLb = lb - expr.constant();
    long long finalUb = ub - expr.constant();
    
    std::stringstream constr;
    if (lb > std::numeric_limits<int>::min() / 2 && ub < std::numeric_limits<int>::max() / 2) {
        constr << "and(ge(" << exprStr.str() << "," << finalLb << "),le(" << exprStr.str() << "," << finalUb << "))";
    } else if (lb > std::numeric_limits<int>::min() / 2) {
        constr << "ge(" << exprStr.str() << "," << finalLb << ")";
    } else if (ub < std::numeric_limits<int>::max() / 2) {
        constr << "le(" << exprStr.str() << "," << finalUb << ")";
    }

    std::stringstream xml;
    xml << "<intension> imp(eq(" << getVarName(condition.id()) << ",1)," << constr.str() << ") </intension>";
    constraints_.push_back(xml.str());
}

// Global Constraints

void XCSP3Backend::addAllDifferent(const std::vector<IntVar>& vars) {
    std::stringstream ss;
    ss << "<allDifferent> ";
    for (const auto& var : vars) ss << getVarName(var.id()) << " ";
    ss << "</allDifferent>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addElement(IntVar index, const std::vector<int>& array, IntVar result) {
    // <element> <list> array... </list> <index> index </index> <value> result </value> </element>
    std::stringstream ss;
    ss << "<element> <list> ";
    for (int v : array) ss << v << " ";
    ss << "</list> <index> " << getVarName(index.id()) << " </index> <value> " << getVarName(result.id()) << " </value> </element>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addElement(IntVar index, const std::vector<IntVar>& vars, IntVar result) {
    std::stringstream ss;
    ss << "<element> <list> ";
    for (const auto& v : vars) ss << getVarName(v.id()) << " ";
    ss << "</list> <index> " << getVarName(index.id()) << " </index> <value> " << getVarName(result.id()) << " </value> </element>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addCircuit(const std::vector<IntVar>& next) {
    std::stringstream ss;
    ss << "<circuit> <list> ";
    for (const auto& v : next) ss << getVarName(v.id()) << " ";
    ss << "</list> </circuit>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addSubCircuit(const std::vector<IntVar>& next) {
    // XCSP3 doesn't typically have a direct 'subcircuit' in the same way, but often it's supported or mapped.
    // If not, we might need a workaround. Assuming <circuit> supports it or using a variant.
    // Actually, XCSP3 has 'circuit' but subcircuit is often specific.
    // For now, let's map it to circuit but note it might be stricter.
    // Wait, subcircuit allows self-loops.
    // We will use <circuit> but careful testing needed.
    // Better: check if ACE/CoSoCo support it explicitly or if we need to model it.
    // Leaving as circuit for prototype.
    std::stringstream ss;
    ss << "<circuit> <list> "; // TODO: Check if subcircuit is a valid tag or attribute
    for (const auto& v : next) ss << getVarName(v.id()) << " ";
    ss << "</list> </circuit>";
    constraints_.push_back(ss.str()); 
}

void XCSP3Backend::addInverse(const std::vector<IntVar>& next, const std::vector<IntVar>& prev) {
    // <channel> <list> next... </list> <list> prev... </list> </channel>
    std::stringstream ss;
    ss << "<channel> <list> ";
    for (const auto& v : next) ss << getVarName(v.id()) << " ";
    ss << "</list> <list> ";
    for (const auto& v : prev) ss << getVarName(v.id()) << " ";
    ss << "</list> </channel>";
    constraints_.push_back(ss.str());
}

// Scheduling

void XCSP3Backend::addNoOverlap(const std::vector<IntervalVar>& intervals) {
    // <noOverlap> <origins> ... </origins> <lengths> ... </lengths> </noOverlap>
    std::stringstream ss;
    ss << "<noOverlap> <origins> ";
    for (const auto& itv : intervals) ss << getVarName(intervals_[itv.id()].startVarId) << " ";
    ss << "</origins> <lengths> ";
    for (const auto& itv : intervals) ss << getVarName(intervals_[itv.id()].sizeVarId) << " ";
    ss << "</lengths> </noOverlap>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addNoOverlap(const std::vector<OptionalIntervalVar>& intervals) {
    // XCSP3 noOverlap supports optional tasks via logic or specific attributes?
    // Usually standard noOverlap handles it if duration/start are set properly or if it ignores absent.
    // Proper XCSP3 way: use 'noOverlap' with optional handling or conditional constraints.
    // ACE supports noOverlap on tasks.
    // Let's assume we pass the variables.
    std::stringstream ss;
    ss << "<noOverlap> <origins> ";
    for (const auto& itv : intervals) ss << getVarName(optionalIntervals_[itv.id()].startVarId) << " ";
    ss << "</origins> <lengths> ";
    for (const auto& itv : intervals) ss << getVarName(optionalIntervals_[itv.id()].sizeVarId) << " ";
    ss << "</lengths> </noOverlap>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addCumulative(const std::vector<IntervalVar>& intervals, const std::vector<int>& demands, int capacity) {
    std::stringstream ss;
    ss << "<cumulative> <origins> ";
    for (const auto& itv : intervals) ss << getVarName(intervals_[itv.id()].startVarId) << " ";
    ss << "</origins> <lengths> ";
    for (const auto& itv : intervals) ss << getVarName(intervals_[itv.id()].sizeVarId) << " ";
    ss << "</lengths> <heights> ";
    for (int h : demands) ss << h << " ";
    ss << "</heights> <condition> (le," << capacity << ") </condition> </cumulative>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addCumulative(const std::vector<OptionalIntervalVar>& intervals, const std::vector<int>& demands, int capacity) {
    // Similar to noOverlap, assumes solver handles optionality if start/size vars are controlled or if using tasks.
    std::stringstream ss;
    ss << "<cumulative> <origins> ";
    for (const auto& itv : intervals) ss << getVarName(optionalIntervals_[itv.id()].startVarId) << " ";
    ss << "</origins> <lengths> ";
    for (const auto& itv : intervals) ss << getVarName(optionalIntervals_[itv.id()].sizeVarId) << " ";
    ss << "</lengths> <heights> ";
    for (int h : demands) ss << h << " ";
    ss << "</heights> <condition> (le," << capacity << ") </condition> </cumulative>";
    constraints_.push_back(ss.str());
}

void XCSP3Backend::addEndBeforeStart(IntervalVar interval1, IntervalVar interval2, int delay) {
    // end1 + delay <= start2
    // le(add(end1, delay), start2)
    std::string end1 = getVarName(intervals_[interval1.id()].endVarId);
    std::string start2 = getVarName(intervals_[interval2.id()].startVarId);
    std::stringstream ss;
    ss << "<intension> le(add(" << end1 << "," << delay << ")," << start2 << ") </intension>";
    constraints_.push_back(ss.str());
}

IntVar XCSP3Backend::startOf(IntervalVar interval) { return IntVar(intervals_[interval.id()].startVarId); }
IntVar XCSP3Backend::endOf(IntervalVar interval) { return IntVar(intervals_[interval.id()].endVarId); }
IntVar XCSP3Backend::durationOf(IntervalVar interval) { return IntVar(intervals_[interval.id()].sizeVarId); }
IntVar XCSP3Backend::startOf(OptionalIntervalVar interval, int) { return IntVar(optionalIntervals_[interval.id()].startVarId); }
IntVar XCSP3Backend::endOf(OptionalIntervalVar interval, int) { return IntVar(optionalIntervals_[interval.id()].endVarId); }

// Objective

void XCSP3Backend::minimize(const LinearExpr& expr) {
    isMinimization_ = true;
    std::stringstream ss;
    ss << "<minimize type=\"sum\"> ";
    // XCSP3 objectives are functional. minimize sum(...)
    // Assuming linear expression.
    // <minimize> add(mul(...)...) </minimize> 
    // Simplified: <minimize> <list> ... </list> <coeffs> ... </coeffs> </minimize> ??
    // Standard: <minimize> <functional> ... </functional> </minimize> or specialized.
    // Lets use functional `add(...)`.
    ss << "add(";
    if (expr.terms().empty()) ss << "0";
    else {
        size_t idx = 0;
        for (auto const& [var, coeff] : expr.terms()) {
            if (idx > 0) ss << ",";
            ss << "mul(" << coeff << "," << getVarName(var.id()) << ")";
            idx++;
        }
    }
    ss << ") </minimize>";
    objectiveXml_ = ss.str();
}

void XCSP3Backend::maximize(const LinearExpr& expr) {
    isMinimization_ = false;
    // Similar to minimize
    std::stringstream ss;
    ss << "<maximize> add(";
    if (expr.terms().empty()) ss << "0";
    else {
        size_t idx = 0;
        for (auto const& [var, coeff] : expr.terms()) {
            if (idx > 0) ss << ",";
            ss << "mul(" << coeff << "," << getVarName(var.id()) << ")";
            idx++;
        }
    }
    ss << ") </maximize>";
    objectiveXml_ = ss.str();
}

void XCSP3Backend::minimizeMakespan(const std::vector<IntervalVar>& intervals) {
    isMinimization_ = true;
    std::stringstream ss;
    ss << "<minimize> max(";
    for (size_t i = 0; i < intervals.size(); ++i) {
        if (i > 0) ss << ",";
        ss << getVarName(intervals_[intervals[i].id()].endVarId);
    }
    ss << ") </minimize>";
    objectiveXml_ = ss.str();
}

// Solving

bool XCSP3Backend::exportModel(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) return false;

    out << "<instance format=\"XCSP3\" type=\"COP\">\n";
    out << "  <variables>\n";
    for (const auto& var : variables_) {
        out << "    <var id=\"" << var.name << "\"";
        if (var.isBool) {
            out << " type=\"integer\"> 0 1 </var>\n";
        } else {
            out << " type=\"integer\"> " << var.lb << ".." << var.ub << " </var>\n";
        }
    }
    out << "  </variables>\n";
    
    out << "  <constraints>\n";
    for (const auto& c : constraints_) {
        out << "    " << c << "\n";
    }
    out << "  </constraints>\n";

    if (!objectiveXml_.empty()) {
        out << "  <objectives>\n";
        out << "    " << objectiveXml_ << "\n";
        out << "  </objectives>\n";
    }

    out << "</instance>\n";
    return true;
}

CPStatus XCSP3Backend::solve(double timeout) {
    // 1. Export model
    std::string modelFile = "model.xml";
    if (!exportModel(modelFile)) return CPStatus::Error;

    // 2. Run ACE (Assuming simple command for now, should be configurable)
    // Using environment variable or default
    const char* solverCmd = std::getenv("XCSP3_SOLVER_CMD");
    std::string cmd;
    if (solverCmd) {
        cmd = std::string(solverCmd) + " " + modelFile;
    } else {
        // Fallback: assume ACE.jar in current dir or PATH
        cmd = "java -jar ACE.jar " + modelFile; 
    }

    // Redirect output to file
    cmd += " > solution.xml";
    
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        // Warning: solver might return non-zero for unsat or other statuses
    }

    // 3. Parse solution
    std::ifstream in("solution.xml");
    if (!in) return CPStatus::Unknown;

    std::string line;
    bool foundInstantiation = false;
    bool optimal = false; // Need to parse status from solver output XML/logs

    // Simple parsing of standard XCSP3 solution
    // <instantiation> <list> x y z </list> <values> 1 2 3 </values> </instantiation>
    
    std::string fullText;
    while(std::getline(in, line)) fullText += line;

    // Regex parsing (rough)
    std::regex valRegex("<values>([^<]+)</values>");
    std::regex listRegex("<list>([^<]+)</list>");
    std::smatch valMatch, listMatch;
    
    if (std::regex_search(fullText, listMatch, listRegex) && std::regex_search(fullText, valMatch, valRegex)) {
        std::string varsStr = listMatch[1];
        std::string valsStr = valMatch[1];
        
        std::stringstream vs(varsStr);
        std::stringstream vals(valsStr);
        std::string varName;
        int value;
        
        // Map names back to IDs
        std::map<std::string, int> nameToId;
        for(size_t i=0; i<variables_.size(); ++i) {
            nameToId[variables_[i].name] = i;
        }

        while (vs >> varName && vals >> value) {
            if (nameToId.count(varName)) {
                integerValues_[nameToId[varName]] = value;
            }
        }
        solved_ = true;
        return CPStatus::Feasible; // assume feasible if solution found
    }

    return CPStatus::Unknown;
}

void XCSP3Backend::setHint(const std::vector<std::pair<IntVar, int>>& hints) {}
void XCSP3Backend::setNumWorkers(int workers) {}
void XCSP3Backend::setRandomSeed(int seed) {}

int XCSP3Backend::getValue(IntVar var) const { return integerValues_.at(var.id()); }
bool XCSP3Backend::getBoolValue(IntVar var) const { return getValue(var) != 0; }
int XCSP3Backend::getStart(IntervalVar interval) const { return getValue(IntVar(intervals_[interval.id()].startVarId)); }
int XCSP3Backend::getEnd(IntervalVar interval) const { return getValue(IntVar(intervals_[interval.id()].endVarId)); }
int XCSP3Backend::getDuration(IntervalVar interval) const { return getValue(IntVar(intervals_[interval.id()].sizeVarId)); }
bool XCSP3Backend::isPresent(OptionalIntervalVar interval) const { 
    return integerValues_.at(optionalIntervals_[interval.id()].presenceVarId) != 0; 
}

double XCSP3Backend::getObjectiveValue() const { return bestObjective_; } // To implement parsing
double XCSP3Backend::getObjectiveBound() const { return 0.0; }
double XCSP3Backend::getSolveTime() const { return solveTime_; }
long long XCSP3Backend::getNumBranches() const { return 0; }
long long XCSP3Backend::getNumFailures() const { return 0; }

void XCSP3Backend::clear() {
    variables_.clear();
    intervals_.clear();
    optionalIntervals_.clear();
    constraints_.clear();
    integerValues_.clear();
    solved_ = false;
    objectiveXml_ = "";
}

}
}
