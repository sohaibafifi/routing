// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Problem.hpp"

#include <plugins/readers/ReaderCorePlugin/Reader.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace composable::top {
namespace detail {

inline std::string trim(const std::string& value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

inline int parseVehiclesFromName(const std::string& name) {
    for (size_t i = 0; i < name.size(); ++i) {
        if (name[i] != 'k' && name[i] != 'K') {
            continue;
        }
        size_t j = i + 1;
        if (j >= name.size() || !std::isdigit(static_cast<unsigned char>(name[j]))) {
            continue;
        }
        int value = 0;
        while (j < name.size() && std::isdigit(static_cast<unsigned char>(name[j]))) {
            value = value * 10 + (name[j] - '0');
            ++j;
        }
        return value;
    }
    return 0;
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
        std::string name;
        int dimension = 0;
        routing::Capacity capacity = 0.0;
        int numVehicles = 0;
        std::map<int, std::pair<routing::Duration, routing::Duration>> coords;
        std::map<int, routing::Demand> demands;
        std::vector<int> depotIds;

        while (std::getline(file, line)) {
            line = detail::trim(line);
            if (line.empty()) {
                continue;
            }

            if (line == "EOF") {
                break;
            }

            auto colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = detail::trim(line.substr(0, colonPos));
                std::string value = detail::trim(line.substr(colonPos + 1));

                if (key == "NAME") {
                    name = value;
                    problem->setName(value);
                } else if (key == "DIMENSION") {
                    dimension = std::stoi(value);
                } else if (key == "CAPACITY") {
                    capacity = std::stod(value);
                } else if (key == "VEHICLES" || key == "NUM_VEHICLES") {
                    numVehicles = std::stoi(value);
                }
            } else if (line == "NODE_COORD_SECTION") {
                for (int i = 0; i < dimension && std::getline(file, line); ++i) {
                    std::istringstream iss(line);
                    int id = 0;
                    routing::Duration x = 0.0;
                    routing::Duration y = 0.0;
                    if (iss >> id >> x >> y) {
                        coords[id] = {x, y};
                    }
                }
            } else if (line == "DEMAND_SECTION") {
                for (int i = 0; i < dimension && std::getline(file, line); ++i) {
                    std::istringstream iss(line);
                    int id = 0;
                    routing::Demand demand = 0;
                    if (iss >> id >> demand) {
                        demands[id] = demand;
                    }
                }
            } else if (line == "DEPOT_SECTION") {
                while (std::getline(file, line)) {
                    line = detail::trim(line);
                    if (line.empty()) {
                        continue;
                    }
                    int depotId = std::stoi(line);
                    if (depotId == -1) {
                        break;
                    }
                    depotIds.push_back(depotId);
                }
            }
        }

        if (problem->getName().empty() && !name.empty()) {
            problem->setName(name);
        }

        if (numVehicles == 0 && !name.empty()) {
            numVehicles = detail::parseVehiclesFromName(name);
        }
        if (numVehicles == 0) {
            if (dimension > 0) {
                numVehicles = static_cast<int>(std::ceil(dimension / 10.0));
            } else {
                numVehicles = 1;
            }
        }

        if (depotIds.empty() && !coords.empty()) {
            depotIds.push_back(coords.begin()->first);
        }

        for (int k = 0; k < numVehicles; ++k) {
            problem->addVehicle(k, capacity);
        }

        for (const auto& [id, coord] : coords) {
            if (std::find(depotIds.begin(), depotIds.end(), id) != depotIds.end()) {
                problem->addDepot(id, coord.first, coord.second);
            } else {
                routing::Demand demand = 0;
                auto demandIt = demands.find(id);
                if (demandIt != demands.end()) {
                    demand = demandIt->second;
                }
                problem->addClient(id, coord.first, coord.second, demand, demand);
            }
        }

        problem->syncLegacyPointers();
        return problem;
    }
};

} // namespace composable::top
