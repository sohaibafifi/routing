#include "gtest/gtest.h"
#include "plugins/solvers/XCSP3SolverPlugin/XCSP3Backend.hpp"
#include "plugins/solvers/XCSP3SolverPlugin/XCSP3Solver.hpp"
#include "core/Problem.hpp"
#include "core/interfaces/ICPConstraintGenerator.hpp"
#include <fstream>
#include <filesystem>
#include <regex>

using namespace routing;
using namespace routing::cp;

// Mock Generator to simulate a Capacity Plugin for CP
class TestCapacityGenerator : public ICPConstraintGenerator {
public:
    std::string name() const override { return "TestCapacityGenerator"; }
    std::vector<AttributeTypeId> requiredAttributes() const override { return {}; } // Simplify for test

    void addVariables(ICPBackend& cp, Problem& problem) override {
        // Create vehicle capacity variables or flow variables
        // For test, let's create a visualizable set:
        // x_i in [1..10]
        for(int i=0; i<3; ++i) {
             cp.newIntVar(1, 10, "x_" + std::to_string(i));
        }
    }

    void addConstraints(ICPBackend& cp, Problem& problem) override {
        // Add a constraint sum(x) <= 20
        LinearExpr sumExpr;
        for(int i=0; i<3; ++i) {
            // Re-create the var to reference it given we know IDs are sequential 0,1,2
            // In real code we'd store creation results or look them up.
            // XCSP3Backend IDs are 0-based.
            sumExpr.addTerm(IntVar(i), 1);
        }
        cp.addLinearConstraint(sumExpr, 0, 20); // 0 <= sum <= 20
    }
    
    void addObjectiveTerms(ICPBackend& cp, Problem& problem, LinearExpr& expr) override {
        // Minimize sum
        for(int i=0; i<3; ++i) expr.addTerm(IntVar(i), 1);
    }
};

TEST(XCSP3SolverTest, RealExampleWithGenerator) {
    Problem problem;
    // No need to add attributes as our mock generator doesn't check them strictly
    
    XCSP3Solver solver(&problem);
    
    // Inject our custom generator
    solver.addGenerator(std::make_unique<TestCapacityGenerator>());
    
    // Test Solve Generation
    // We expect "model.xml" to be generated with specific content
    
    // Capture output to keep test clean
    testing::internal::CaptureStdout();
    
    // Set environment variable to prevent trying to run actual java/ace if not present
    setenv("XCSP3_SOLVER_CMD", "echo 'Simulate Solver'", 1);
    
    solver.solve(0.1); 
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Verify file exists
    ASSERT_TRUE(std::filesystem::exists("model.xml"));
    
    // Read and verify content
    std::ifstream in("model.xml");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    
    // Check variables
    EXPECT_TRUE(content.find("<var id=\"x_0\" type=\"integer\"> 1..10 </var>") != std::string::npos);
    EXPECT_TRUE(content.find("<var id=\"x_2\" type=\"integer\"> 1..10 </var>") != std::string::npos);

    // Check constraint: sum(x) <= 20
    // XCSP3Backend format: <intension> and(ge(add(mul(1,x_0),mul(1,x_1),mul(1,x_2)),0),le(add(mul(1,x_0),mul(1,x_1),mul(1,x_2)),20)) </intension>
    // Or simplified if logic changes. Regex is safer for partial match.
    EXPECT_TRUE(content.find("le(add(mul(1,x_0),mul(1,x_1),mul(1,x_2)),20)") != std::string::npos);
    
    // Check objective
    // <minimize type="sum"> add(mul(1,x_0),mul(1,x_1),mul(1,x_2)) </minimize>
    EXPECT_TRUE(content.find("<minimize type=\"sum\">") != std::string::npos);

    // Cleanup
    if (std::filesystem::exists("model.xml")) std::filesystem::remove("model.xml");
    if (std::filesystem::exists("solution.xml")) std::filesystem::remove("solution.xml");
}
