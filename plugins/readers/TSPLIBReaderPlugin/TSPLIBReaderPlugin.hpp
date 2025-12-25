// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/IReader.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <map>

namespace routing {
namespace plugins {

namespace tsplib_detail {
    inline std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }

    inline std::string trim(const std::string& str) {
        const auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        const auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    inline bool hasExtension(const std::string& filepath,
                             const std::vector<std::string>& extensions) {
        auto pos = filepath.find_last_of('.');
        if (pos == std::string::npos) return false;
        std::string ext = toLower(filepath.substr(pos));
        for (const auto& candidate : extensions) {
            if (ext == toLower(candidate)) {
                return true;
            }
        }
        return false;
    }
}

/**
 * @brief Standalone TSPLIB/CVRPLIB format reader using composable API
 *
 * Reads standard TSPLIB-format CVRP benchmark instances.
 * Supports NODE_COORD_SECTION, DEMAND_SECTION, and DEPOT_SECTION.
 */
class TSPLIBReader : public IReader {
public:
    std::string formatName() const override { return "tsplib"; }

    std::vector<std::string> supportedExtensions() const override {
        return {".vrp", ".tsp"};
    }

    bool canRead(const std::string& filepath) const override {
        return tsplib_detail::hasExtension(filepath, supportedExtensions());
    }

    Problem* readFile(const std::string& filepath) override {
        detectedType_ = "cvrp";

        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        auto* problem = new Problem();

        // Enable CVRP attributes
        problem->enableAttributes<
            attributes::GeoNode,
            attributes::Consumer,
            attributes::Stock
        >();

        std::string line;
        int dimension = 0;
        double capacity = 0;
        int numVehicles = 0;
        std::map<int, std::pair<double, double>> coords;
        std::map<int, double> demands;
        std::vector<int> depotIds;

        // Parse header and sections
        while (std::getline(file, line)) {
            line = tsplib_detail::trim(line);
            if (line.empty()) continue;

            // Check for key-value pairs
            auto colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = tsplib_detail::trim(line.substr(0, colonPos));
                std::string value = tsplib_detail::trim(line.substr(colonPos + 1));

                if (key == "NAME") {
                    problem->setName(value);
                } else if (key == "DIMENSION") {
                    dimension = std::stoi(value);
                } else if (key == "CAPACITY") {
                    capacity = std::stod(value);
                } else if (key == "VEHICLES" || key == "NUM_VEHICLES") {
                    numVehicles = std::stoi(value);
                }
            }
            // Check for section headers
            else if (line == "NODE_COORD_SECTION") {
                for (int i = 0; i < dimension && std::getline(file, line); ++i) {
                    std::istringstream iss(line);
                    int id;
                    double x, y;
                    if (iss >> id >> x >> y) {
                        coords[id] = {x, y};
                    }
                }
            } else if (line == "DEMAND_SECTION") {
                for (int i = 0; i < dimension && std::getline(file, line); ++i) {
                    std::istringstream iss(line);
                    int id;
                    double demand;
                    if (iss >> id >> demand) {
                        demands[id] = demand;
                    }
                }
            } else if (line == "DEPOT_SECTION") {
                while (std::getline(file, line)) {
                    int depotId = std::stoi(tsplib_detail::trim(line));
                    if (depotId == -1) break;
                    depotIds.push_back(depotId);
                }
            } else if (line == "EOF") {
                break;
            }
        }

        // Default depot if not specified
        if (depotIds.empty() && !coords.empty()) {
            depotIds.push_back(coords.begin()->first);
        }

        // Default number of vehicles if not specified
        if (numVehicles == 0) {
            numVehicles = static_cast<int>(std::ceil(dimension / 10.0));  // Rough estimate
        }

        // Create vehicles
        for (int k = 0; k < numVehicles; ++k) {
            auto* vehicle = problem->addVehicle(k);
            vehicle->addAttribute<attributes::Stock>(capacity);
        }

        // Create depot and clients
        for (const auto& [id, coord] : coords) {
            if (std::find(depotIds.begin(), depotIds.end(), id) != depotIds.end()) {
                auto* depot = problem->addDepot(id);
                depot->addAttribute<attributes::GeoNode>(coord.first, coord.second);
            } else {
                auto* client = problem->addClient(id);
                client->addAttribute<attributes::GeoNode>(coord.first, coord.second);
                if (demands.count(id)) {
                    client->addAttribute<attributes::Consumer>(demands[id]);
                }
            }
        }

        // Sync legacy pointer arrays for compatibility
        problem->syncLegacyPointers();

        return problem;
    }

    std::string detectedProblemType() const override {
        return detectedType_;
    }

private:
    std::string detectedType_ = "unknown";
};

class TSPLIBReaderPlugin : public IPlugin {
public:
    std::string name() const override { return "TSPLIBReaderPlugin"; }
    PluginType type() const override { return PluginType::Reader; }

    void initialize(PluginRegistry& registry) override {
        registry.registerReader(
            "tsplib",
            {".vrp", ".tsp"},
            []() -> std::unique_ptr<IReader> {
                return std::make_unique<TSPLIBReader>();
            });
    }
};

} // namespace plugins
} // namespace routing
