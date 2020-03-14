// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <type_traits>

namespace routing {
    class IEntityData {
    };

    class ISolutionValue {
    };

    template<typename V>
    class EntityData : public IEntityData {
    public :
        EntityData(const V &p_val) : val(p_val) {}

        template<typename T>
        bool is() { return std::is_same<V, T>::value(); }

        V getValue() const { return val; }

    private :
        V val;
    };

    template<typename V>
    class SolutionValue : public ISolutionValue {
    public :
        SolutionValue(const V &p_val) : val(p_val) {}

        template<typename T>
        bool is() { return std::is_same<V, T>::value(); }

        V getValue() const { return val; }

        void setValue(V v) { this->val = v; }

    private :
        V val;
    };
}
