// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <core/data/attributes.hpp>
#include <utility>
#include <vector>

namespace vrptwtd {

    namespace attributes {


        struct Synced : public routing::Model{
            Synced(std::vector<Synced*> p_brothers,
                    std::vector<routing::Duration> p_deltas) :
                    brothers(std::move(p_brothers)),
                    deltas(std::move(p_deltas)) {
            }

            std::vector<Synced*> brothers;
            std::vector<routing::Duration> deltas;

            routing::Duration getDelta(unsigned i) const { return this->deltas.at(i); }
            Synced* getBrother(unsigned i) const { return this->brothers.at(i); }
            unsigned long getBrothersCount() const { return this->brothers.size(); }

            void addBrother(Synced* brother, routing::Duration delta) {
                this->brothers.push_back(brother);
                this->deltas.push_back(delta);
            }

        };

    }
}
