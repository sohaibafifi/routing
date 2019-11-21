//
// Created by Sohaib LAFIFI on 20/11/2019.
//
#pragma once

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <stack>
#include <algorithm>


namespace Utilities {
    inline std::string filename(std::string path) {
        std::string filename;
#ifdef _WIN32
        size_t pos = path.find_last_of("\\");
#else
        size_t pos = path.find_last_of("/");
#endif
        if (pos != std::string::npos)
            filename.assign(path.begin() + pos + 1, path.end());
        else
            filename = path;
        return filename.substr(0, filename.find_last_of('.'));
    }

    inline std::vector<std::string> splitString(std::string line, char sep) {
        std::vector<std::string> split;
        split.clear();
        size_t pos;
        do {
            pos = line.find_first_of(sep);
            if (pos != 0)
                split.push_back(line.substr(0, pos));
            line = line.substr(pos + 1);
        } while (pos < line.npos);
        return split;
    }

    inline std::string itos(int i) // convert int to std::string
    {
        std::stringstream s;
        s << i;
        return s.str();
    }

    inline std::string itos(unsigned i) // convert int to std::string
    {
        std::stringstream s;
        s << i;
        return s.str();
    }

    inline bool less_vectors(const std::vector<unsigned int> &a, const std::vector<unsigned int> &b) {
        return a.size() < b.size();
    }

#ifdef _WIN32
    inline   int round_(float x) { return (floor(x + 0.5)); }
    inline   int round_(double x) { return (floor(x + 0.5)); }
    inline   int trunc_(float x) { return (x>0) ? floor(x) : ceil(x) ; }
    inline   int trunc_(double x) { return (x>0) ? floor(x) : ceil(x) ; }
#endif

    inline void debug(std::string message) {
#ifdef _DEBUG
        std::cout << message.c_str() << std::endl;
#endif
    }

}

