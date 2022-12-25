// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <map>
#include <string>
#include <exception>
#include <utility>

namespace routing {
    class ParameterNotFound : public std::exception {
        std::string message;

    public :
        explicit ParameterNotFound(const std::string& p_name)  {
            message = std::string(std::string("Parameter ") + p_name + std::string(" not found "));
        }

        const char *what() const noexcept override {
            return this->message.c_str();
        }
    };

    class Configuration {
        std::map<std::string, int> intParams;
        std::map<std::string, double> doubleParams;
        std::map<std::string, bool> boolParams;
    public :

        int getIntParam(const std::string& id) {
            auto element = this->intParams.find(id);
            if (element == this->intParams.end()) {
                throw  ParameterNotFound(id);
            } else return element->second;
        }

        bool getBoolParam(const std::string& id) {
            auto element = this->boolParams.find(id);
            if (element == this->boolParams.end()) {
                throw  ParameterNotFound(id);
            } else return element->second;
        }

        double getDoubleParam(const std::string& id) {
            auto element = this->doubleParams.find(id);
            if (element == this->doubleParams.end()) {
                throw  ParameterNotFound(id);
            } else return element->second;
        }


        void setIntParam(const std::string& id, int value) {
            this->intParams[id] = value;
        }

        void setBoolParam(const std::string& id, bool value) {
            this->boolParams[id] = value;
        }

        void setDoubleParam(const std::string& id, double value) {
            this->doubleParams[id] = value;
        }
    };
}

