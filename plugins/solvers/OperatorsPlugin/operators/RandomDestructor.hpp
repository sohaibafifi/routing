// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Destructor.hpp"
#include <algorithm>
#include <random>
#include <utility>
#include <vector>

namespace routing {

    class RandomDestructor : public Destructor {
    public:
        explicit RandomDestructor(double fraction = 0.2)
            : fraction_(fraction) {}

        void destruct(Solution *solution) override {
            if (!solution) {
                return;
            }

            std::vector<std::pair<unsigned long, unsigned long>> positions;
            for (unsigned long t = 0; t < solution->getNbTour(); ++t) {
                auto* tour = solution->getTour(t);
                if (!tour) {
                    continue;
                }
                for (unsigned long p = 0; p < tour->getNbClient(); ++p) {
                    positions.emplace_back(t, p);
                }
            }

            if (positions.empty()) {
                return;
            }

            size_t removeCount = static_cast<size_t>(positions.size() * fraction_);
            if (removeCount < 1) {
                removeCount = 1;
            }
            if (removeCount > positions.size()) {
                removeCount = positions.size();
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::shuffle(positions.begin(), positions.end(), gen);
            positions.resize(removeCount);

            std::sort(positions.begin(), positions.end(),
                      [](const auto& a, const auto& b) {
                          if (a.first != b.first) {
                              return a.first < b.first;
                          }
                          return a.second > b.second;
                      });

            for (const auto& pos : positions) {
                solution->removeClient(pos.first, pos.second);
            }
        }

    private:
        double fraction_;
    };

}
