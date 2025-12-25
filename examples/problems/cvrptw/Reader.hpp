// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Problem.hpp"

#include <plugins/readers/ReaderCorePlugin/Reader.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace composable::cvrptw {
namespace detail {

inline std::string trim(const std::string& value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

inline bool isBlank(const std::string& value) {
    return trim(value).empty();
}

} // namespace detail

class Reader : public routing::Reader {
public:
    routing::Problem* readFile(const std::string& filepath) override {
        auto* problem = new Problem();
        std::ifstream file(filepath);
        if (!file.is_open()) {
            delete problem;
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        std::string line;
        if (!std::getline(file, line)) {
            delete problem;
            throw std::runtime_error("Empty file: " + filepath);
        }
        problem->setName(detail::trim(line));

        for (int i = 0; i < 3 && std::getline(file, line); ++i) {
        }

        int numVehicles = 0;
        routing::Capacity capacity = 0.0;
        if (std::getline(file, line)) {
            std::istringstream vehicleLine(line);
            vehicleLine >> numVehicles >> capacity;
        }

        for (int k = 0; k < numVehicles; ++k) {
            problem->addVehicle(k, capacity);
        }

        for (int i = 0; i < 4 && std::getline(file, line); ++i) {
        }

        bool firstNode = true;
        while (std::getline(file, line)) {
            if (detail::isBlank(line)) {
                continue;
            }

            std::istringstream iss(line);
            unsigned id = 0;
            routing::Duration x = 0.0;
            routing::Duration y = 0.0;
            routing::Demand demand = 0;
            routing::Duration readyTime = 0.0;
            routing::Duration dueDate = 0.0;
            routing::Duration serviceTime = 0.0;

            if (!(iss >> id >> x >> y >> demand >> readyTime >> dueDate >> serviceTime)) {
                continue;
            }

            if (firstNode) {
                problem->addDepot(id, x, y, readyTime, dueDate);
                firstNode = false;
            } else {
                problem->addClient(id, x, y, demand, readyTime, dueDate, serviceTime);
            }
        }

        problem->syncLegacyPointers();
        return problem;
    }
};

} // namespace composable::cvrptw
