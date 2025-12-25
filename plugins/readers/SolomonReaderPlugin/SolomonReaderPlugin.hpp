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
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

namespace routing {
namespace plugins {

namespace solomon_detail {
    inline std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
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
 * @brief Standalone Solomon format reader using composable API
 *
 * Reads standard Solomon CVRPTW benchmark instances.
 * Format:
 *   Line 1: Instance name
 *   Lines 2-4: Empty/header
 *   Line 5: Number of vehicles, vehicle capacity
 *   Lines 6-9: Empty/header
 *   Line 10+: CUST NO. XCOORD. YCOORD. DEMAND READY TIME DUE DATE SERVICE TIME
 */
class SolomonReader : public IReader {
public:
    std::string formatName() const override { return "solomon"; }

    std::vector<std::string> supportedExtensions() const override {
        return {".txt", ".sol"};
    }

    bool canRead(const std::string& filepath) const override {
        return solomon_detail::hasExtension(filepath, supportedExtensions());
    }

    Problem* readFile(const std::string& filepath) override {
        detectedType_ = "cvrptw";

        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        auto* problem = new Problem();

        // Enable CVRPTW attributes
        problem->enableAttributes<
            attributes::GeoNode,
            attributes::Consumer,
            attributes::Stock,
            attributes::Rendezvous,
            attributes::ServiceQuery
        >();

        std::string line;

        // Line 1: Instance name
        std::getline(file, line);
        problem->setName(line);

        // Lines 2-4: Skip empty/header lines
        for (int i = 0; i < 3; ++i) {
            std::getline(file, line);
        }

        // Line 5: Number of vehicles, vehicle capacity
        int numVehicles = 0;
        double capacity = 0;
        std::getline(file, line);
        std::istringstream vehicleLine(line);
        vehicleLine >> numVehicles >> capacity;

        // Create vehicles
        for (int k = 0; k < numVehicles; ++k) {
            auto* vehicle = problem->addVehicle(k);
            vehicle->addAttribute<attributes::Stock>(capacity);
        }

        // Lines 6-9: Skip empty/header lines
        for (int i = 0; i < 4; ++i) {
            std::getline(file, line);
        }

        // Read customer data
        bool firstNode = true;
        while (std::getline(file, line)) {
            if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
                continue;  // Skip empty lines
            }

            std::istringstream iss(line);
            int id;
            double x, y, demand, readyTime, dueDate, serviceTime;

            if (!(iss >> id >> x >> y >> demand >> readyTime >> dueDate >> serviceTime)) {
                continue;  // Skip malformed lines
            }

            if (firstNode) {
                // First node is the depot
                auto* depot = problem->addDepot(id);
                depot->addAttribute<attributes::GeoNode>(x, y);
                depot->addAttribute<attributes::Rendezvous>(readyTime, dueDate);
                firstNode = false;
            } else {
                // Subsequent nodes are customers
                auto* client = problem->addClient(id);
                client->addAttribute<attributes::GeoNode>(x, y);
                client->addAttribute<attributes::Consumer>(demand);
                client->addAttribute<attributes::Rendezvous>(readyTime, dueDate);
                client->addAttribute<attributes::ServiceQuery>(serviceTime);
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

class SolomonReaderPlugin : public IPlugin {
public:
    std::string name() const override { return "SolomonReaderPlugin"; }
    PluginType type() const override { return PluginType::Reader; }

    void initialize(PluginRegistry& registry) override {
        registry.registerReader(
            "solomon",
            {".txt", ".sol"},
            []() -> std::unique_ptr<IReader> {
                return std::make_unique<SolomonReader>();
            });
    }
};

} // namespace plugins
} // namespace routing
