// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <map>
namespace routing {

    class Memory {
    protected:
        static Memory *singleton;
        std::map<long, double> space;
    public:
        Memory(const Memory &) = delete;
        Memory(){}
        Memory &operator=(const Memory &) = delete;
        std::pair<bool, double> at(long hash){
            std::map<long, double>::iterator it =  space.find(hash);
            if(it != space.end()) return std::make_pair(true, it->second);
             return std::make_pair(false, 0.0);
        }

        void add(long hash, double cost){
            space[hash] = cost;
        }
    };
}

