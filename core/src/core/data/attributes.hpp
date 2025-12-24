// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <type_traits>
#include <typeindex>
#include <string>

namespace routing {

    // Type identifier for attributes
    using AttributeTypeId = std::type_index;

    /**
     * @brief Base interface for all composable attributes
     *
     * Attributes are composable units that can be attached to entities
     * (clients, vehicles, depots) at runtime. Each attribute type is
     * associated with constraint generators that are automatically
     * activated when the attribute is enabled on a problem.
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
     *
     * Provides automatic type identification. Derive from this
     * instead of IAttribute directly.
     *
     * Example:
     *   struct MyAttribute : public Attribute<MyAttribute> { ... };
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
}
