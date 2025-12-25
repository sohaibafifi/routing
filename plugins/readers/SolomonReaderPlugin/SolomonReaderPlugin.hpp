// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/IPlugin.hpp"
#include "core/PluginRegistry.hpp"
#include "core/interfaces/IReader.hpp"
#include "plugins/attributes/ComposableCorePlugin/ComposableProblem.hpp"

#include <cvrptw/Reader.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

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
        return reader_.readFile(filepath);
    }

    std::string detectedProblemType() const override {
        return detectedType_;
    }

private:
    cvrptw::Reader reader_;
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
