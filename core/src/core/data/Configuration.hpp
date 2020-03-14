#pragma once

#include <map>
#include <string>
#include <exception>

namespace routing {
    class ParameterNotFound : public std::exception {
        std::string name;

    public :
        ParameterNotFound(std::string p_name) : name(p_name) {
        }

        virtual const char *what() const throw() {
            return std::string(std::string("Parameter ") + this->name + std::string(" not found ")).c_str();
        }
    };

    class Configuration {
        std::map<std::string, int> intParams;
        std::map<std::string, double> doubleParams;
        std::map<std::string, bool> boolParams;
    public :
        static const std::string ga_itermax;
        static const std::string ma_itermax;

        int getIntParam(std::string id) {
            auto element = this->intParams.find(id);
            if (element == this->intParams.end()) {
                throw new ParameterNotFound(id);
            } else return element->second;
        }

        bool getBoolParam(std::string id) {
            auto element = this->boolParams.find(id);
            if (element == this->boolParams.end()) {
                throw new ParameterNotFound(id);
            } else return element->second;
        }

        double getDoubleParam(std::string id) {
            auto element = this->doubleParams.find(id);
            if (element == this->doubleParams.end()) {
                throw new ParameterNotFound(id);
            } else return element->second;
        }
    };
     const std::string Configuration::ga_itermax = "ga/itermax";
     const std::string Configuration::ma_itermax = "ma/itermax";
}

