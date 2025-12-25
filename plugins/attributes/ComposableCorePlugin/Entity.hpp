// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Model.hpp"
#include "core/interfaces/IAttribute.hpp"
#include "models/Client.hpp"
#include "models/Vehicle.hpp"
#include "models/Depot.hpp"

#include <map>
#include <set>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <utility>

namespace routing {

    /**
     * @brief Runtime-composable entity that can hold any combination of attributes
     *
     * Entity uses a runtime composition model where attributes are stored in a
     * map and accessed dynamically, replacing multiple-inheritance model composition.
     *
     * Usage:
     *   Entity entity(1);
     *   entity.addAttribute<GeoNode>(10.0, 20.0);
     *   entity.addAttribute<Consumer>(15);
     *
     *   if (entity.hasAttribute<Consumer>()) {
     *       auto demand = entity.getAttribute<Consumer>().getDemand();
     *   }
     *
     * Note: Uses virtual inheritance from Model to avoid diamond inheritance issues
     * when combined with models::Client, models::Vehicle, or models::Depot.
     */
    class Entity : public virtual Model {
    public:
        explicit Entity(unsigned id) {
            setID(id);
        }

        virtual ~Entity() = default;

        // Prevent copying (attributes contain unique_ptr)
        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // Allow moving
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

        /**
         * @brief Add an attribute to this entity
         *
         * Creates a new attribute instance with the given constructor arguments.
         * If an attribute of this type already exists, it will be replaced.
         *
         * @tparam Attr The attribute type (must inherit from Attribute<Attr>)
         * @tparam Args Constructor argument types
         * @param args Constructor arguments forwarded to Attr's constructor
         * @return Reference to the newly created attribute
         */
        template<typename Attr, typename... Args>
        Attr& addAttribute(Args&&... args) {
            static_assert(std::is_base_of<IAttribute, Attr>::value,
                "Attr must inherit from IAttribute");

            auto attr = std::make_unique<Attr>(std::forward<Args>(args)...);
            Attr* ptr = attr.get();
            attributes_[std::type_index(typeid(Attr))] = std::move(attr);
            return *ptr;
        }

        /**
         * @brief Check if this entity has a specific attribute type
         */
        template<typename Attr>
        bool hasAttribute() const {
            return attributes_.count(std::type_index(typeid(Attr))) > 0;
        }

        /**
         * @brief Get an attribute by type (throws if not found)
         */
        template<typename Attr>
        Attr& getAttribute() {
            auto it = attributes_.find(std::type_index(typeid(Attr)));
            if (it == attributes_.end()) {
                throw std::runtime_error(
                    std::string("Attribute not found: ") + typeid(Attr).name());
            }
            return static_cast<Attr&>(*it->second);
        }

        /**
         * @brief Get an attribute by type (const version)
         */
        template<typename Attr>
        const Attr& getAttribute() const {
            auto it = attributes_.find(std::type_index(typeid(Attr)));
            if (it == attributes_.end()) {
                throw std::runtime_error(
                    std::string("Attribute not found: ") + typeid(Attr).name());
            }
            return static_cast<const Attr&>(*it->second);
        }

        /**
         * @brief Try to get an attribute (returns nullptr if not found)
         */
        template<typename Attr>
        Attr* tryGetAttribute() {
            auto it = attributes_.find(std::type_index(typeid(Attr)));
            if (it == attributes_.end()) {
                return nullptr;
            }
            return static_cast<Attr*>(it->second.get());
        }

        /**
         * @brief Try to get an attribute (const version)
         */
        template<typename Attr>
        const Attr* tryGetAttribute() const {
            auto it = attributes_.find(std::type_index(typeid(Attr)));
            if (it == attributes_.end()) {
                return nullptr;
            }
            return static_cast<const Attr*>(it->second.get());
        }

        /**
         * @brief Remove an attribute by type
         * @return true if the attribute was removed, false if it didn't exist
         */
        template<typename Attr>
        bool removeAttribute() {
            return attributes_.erase(std::type_index(typeid(Attr))) > 0;
        }

        /**
         * @brief Get all attribute type IDs on this entity
         */
        std::set<AttributeTypeId> getAttributeTypes() const {
            std::set<AttributeTypeId> result;
            for (const auto& pair : attributes_) {
                result.insert(pair.first);
            }
            return result;
        }

        /**
         * @brief Get the number of attributes on this entity
         */
        size_t attributeCount() const {
            return attributes_.size();
        }

        /**
         * @brief Create a deep copy of this entity (including all attributes)
         */
        std::unique_ptr<Entity> clone() const {
            auto copy = std::make_unique<Entity>(getID());
            copy->name = name;
            for (const auto& pair : attributes_) {
                copy->attributes_[pair.first] = pair.second->clone();
            }
            return copy;
        }

    private:
        std::map<AttributeTypeId, std::unique_ptr<IAttribute>> attributes_;
    };

    /**
     * @brief Client entity with runtime attributes
     */
    class Client : public Entity, public models::Client {
    public:
        explicit Client(unsigned id)
            : Entity(id), models::Client(id) {}
    };

    /**
     * @brief Vehicle entity with runtime attributes
     */
    class Vehicle : public Entity, public models::Vehicle {
    public:
        explicit Vehicle(unsigned id)
            : Entity(id), models::Vehicle(id) {}
    };

    /**
     * @brief Depot entity with runtime attributes
     */
    class Depot : public Entity, public models::Depot {
    public:
        explicit Depot(unsigned id)
            : Entity(id), models::Depot(id) {}
    };

    // Backward compatibility aliases (deprecated)
    using ComposableEntity = Entity;
    using ComposableClient = Client;
    using ComposableVehicle = Vehicle;
    using ComposableDepot = Depot;

} // namespace routing
