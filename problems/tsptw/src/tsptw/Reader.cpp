// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.


#include "Reader.hpp"

#include <queue>
#include <random>
#include <fstream>
#include "Problem.hpp"
#include "cvrptw/models/Vehicle.hpp"
#include <filesystem>

routing::Problem *tsptw::Reader::readFile(const std::string & filepath) {
    try {

        std::string file_name = std::filesystem::path(filepath).filename();

        auto * problem = new tsptw::Problem();
        problem->setName(std::filesystem::path(file_name).replace_extension("").filename().string());
        std::fstream fh(filepath.c_str(), std::ios_base::in);
        try {
            if (!fh.good()) {
                std::cerr << "file open error" << std::endl;
                exit(EXIT_FAILURE);
            }
             std::string line;
             getline(fh, line);
             int nbClients;
             std::stringstream(line) >> nbClients;
             // the first client is the depot
             nbClients--;
             problem->vehicles.push_back(new cvrptw::models::Vehicle(0, nbClients));
             routing::TW fake_tw(0, 0);

             problem->depots.push_back(new cvrptw::models::Depot(0, 0, 0, fake_tw));

            for (int i = 1; i <= nbClients; ++i) {
                problem->clients.push_back(new cvrptw::models::Client(i, 0, 0, 1, 0.0, fake_tw));
            }

            problem->distances = std::vector<std::vector<routing::Duration> >(nbClients + 1);
        problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
        problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients + 1);
        for (unsigned i = 0; i < nbClients + 1; ++i) {
            problem->distances[i] = std::vector<routing::Duration>(nbClients + 1);
            getline(fh, line);
            auto values = splitString(line, ' ');
            if(i == 0) {
                for (unsigned j = 1; j < nbClients + 1; ++j)
                    problem->distances_to_depots[0][j - 1] = std::stod(values[j]);
            }else
            {
                 for (unsigned j = 0; j < nbClients + 1; ++j) {
                     if(j == 0)
                            problem->distances_to_depots[0][i - 1] = std::stod(values[j]);
                         else
                             problem->distances[i - 1][j - 1]  = std::stod(values[j]);
                 }
            }
        }
        for (unsigned i = 0; i < nbClients + 1; ++i) {
            getline(fh, line);
            double twopen, twclose;
            std::stringstream(line) >> twopen >> twclose;
             routing::TW tw(twopen, twclose);
            if(i == 0){
               problem->setDepot( new cvrptw::models::Depot(1, 0, 0, tw) );
            }
            else{
                dynamic_cast<cvrptw::models::Client*>(problem->clients[i - 1])->tw = tw;
            }
        }


        } catch (char const* emsg) {
            std::cerr  << emsg << " reading instance file : " << filepath << std::endl;
        }



        return problem;
    }
    catch (const std::string & emsg) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " :" + emsg);
    }

}
