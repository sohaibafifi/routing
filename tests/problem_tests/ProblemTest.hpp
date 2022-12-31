// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include <fstream>
#include "gtest/gtest.h"

class ProblemTest : public ::testing::Test {
public :
    std::ofstream nullstream;

    ProblemTest() : Test() {
        if (!nullstream.is_open())
            nullstream.open("/dev/null", std::ofstream::out | std::ofstream::app);
    }
};
