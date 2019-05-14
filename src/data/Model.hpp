#pragma once
#include "attributes.hpp"
#include <utility>
#include <iostream>
namespace routing {
    struct Model
    {
            unsigned getID() const {
                return id;
            }
            void setID(const unsigned & p_id){
                id = p_id;
            }
            std::string name;
            virtual ~Model();
            inline bool operator==(const Model& other){
                return other.getID() == this->getID();
            }
            inline bool operator!=(const Model& other){
                return other.getID() != this->getID();
            }

        private :
            unsigned int id;
    };
}
