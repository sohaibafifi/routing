// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>

namespace routing {

    // Type identifier for attributes
    using AttributeTypeId = std::type_index;

    // Legacy interfaces for entity data
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

    /**
     * @brief Base interface for all composable attributes
     */
    class IAttribute {
    public:
        virtual ~IAttribute() = default;

        /// Returns the unique type identifier for this attribute
        virtual AttributeTypeId typeId() const = 0;

        /// Creates a deep copy of this attribute
        virtual std::unique_ptr<IAttribute> clone() const = 0;

        /// Returns a human-readable name for this attribute type
        virtual std::string name() const = 0;
    };

    /**
     * @brief CRTP base class for concrete attributes
     */
    template<typename Derived>
    class Attribute : public IAttribute {
    public:
        AttributeTypeId typeId() const override {
            return std::type_index(typeid(Derived));
        }

        std::string name() const override {
            return typeid(Derived).name();
        }
    };

} // namespace routing
