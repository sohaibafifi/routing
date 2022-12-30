//
// Created by Sohaib LAFIFI on 20/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "../data/models/Solution.hpp"
#include "../data/Problem.hpp"
#include "../data/Configuration.hpp"

template<class Reader>
class Solver {

protected :
    routing::Problem *problem{};
    routing::models::Solution *solution{};

public:
    std::string inputFile;
    std::ostream &os;
    routing::Configuration* configuration{};

    explicit Solver(const std::string &p_inputFile, std::ostream &os = std::cout) : inputFile(p_inputFile), os(os) {
        this->problem = Reader().readFile(p_inputFile);

    }

    virtual bool solve(double timeout = 3600) = 0;
    virtual void setDefaultConfiguration() = 0;

    routing::models::Solution *getSolution() const;
    routing::Problem *getProblem() const { return problem; }
    virtual void save() const;

};

template<class Reader>
routing::models::Solution *Solver<Reader>::getSolution() const {
    return solution;
}


template<class Reader>
void Solver<Reader>::save() const {
std::string output_folder = "output/" + std::filesystem::path(this->inputFile).parent_path().string();
    system((std::string("mkdir -p ") + output_folder).c_str());
    std::string output_file =
            output_folder + "/" + std::filesystem::path(this->inputFile).filename().string() + ".result";

    std::ofstream output(output_file);
    output <<
           this->getProblem()->getName()
           << "\t" << getSolution()->getCost()
           << std::endl;
    output.close();
}
