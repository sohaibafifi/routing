// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <string>
#include <vector>

namespace routing {

class Problem;

/**
 * @brief Interface for instance file readers
 */
class IReader {
public:
    virtual ~IReader() = default;

    /// Get the format name (e.g., "solomon", "tsplib")
    virtual std::string formatName() const = 0;

    /// Get supported file extensions
    virtual std::vector<std::string> supportedExtensions() const = 0;

    /// Check if this reader can read the given file
    virtual bool canRead(const std::string& filepath) const = 0;

    /// Read an instance file and create a Problem
    virtual Problem* readFile(const std::string& filepath) = 0;

    /// Get the detected problem type after reading
    virtual std::string detectedProblemType() const { return "unknown"; }
};

} // namespace routing
