// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/IReader.hpp"

#include <cvrp/Reader.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace routing {
namespace plugins {

namespace tsplib_detail {
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
        return reader_.readFile(filepath);
    }

    std::string detectedProblemType() const override {
        return detectedType_;
    }

private:
    cvrp::Reader reader_;
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
