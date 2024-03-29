// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Reader.hpp"

#include <queue>
#include <random>
#include <fstream>
#include "Problem.hpp"
#include "models/Client.hpp"
#include <filesystem>

routing::Problem *vrptwtd::Reader::readFile(const std::string & filepath) {
    try {

        std::string file_name = std::filesystem::path(filepath).filename();
        auto parts = this->splitString(file_name, '_');
        int nbClients = std::stoi(this->splitString(parts[1], '-')[0]);
        std::string solomon_name = parts[0];
        std::transform(solomon_name.begin(),
                 solomon_name.end(),
                 solomon_name.begin(),
                 ::tolower);
        std::string folder  = std::filesystem::path(filepath).parent_path();

        std::string solomon_file = folder + "/../Instances/"
                + std::to_string(nbClients)
                + "/" + solomon_name + ".txt";

        auto * problem = new vrptwtd::Problem(*dynamic_cast<cvrptw::Problem*>(cvrptw::Reader::readFile(solomon_file)));
        problem->setName(std::filesystem::path(file_name).replace_extension("").filename().string());
        // convert the clients to vrptwtd clients
        for (auto & client : problem->clients) {
            client = new vrptwtd::models::Client(
                    dynamic_cast<cvrptw::models::Client*>(client)
                    );

        }

        std::fstream fh(filepath.c_str(), std::ios_base::in);
        try {
            if (!fh.good()) {
                std::cerr << "file open error" << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string line;
            getline(fh, line);
            while(getline(fh, line)){
                unsigned i,j; double pij,pji;
                std::stringstream(line) >> i >> j >> pij >> pji;
                auto client_i = dynamic_cast<vrptwtd::models::Client*>(problem->clients[i - 1]);
                auto client_j = dynamic_cast<vrptwtd::models::Client*>(problem->clients[j - 1]);
                client_i->addBrother(client_j, pij);
                client_j->addBrother(client_i, pji);

            }
        } catch (char const* emsg) {
            std::cerr  << emsg << " reading sync file : " << filepath << std::endl;
        }



        return problem;
    }
    catch (const std::string & emsg) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " :" + emsg);
    }

}
