// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "Model.hpp"
#include "attributes.hpp"
#include "models/Client.hpp"
#include "models/Vehicle.hpp"
#include "models/Depot.hpp"

#include <map>
#include <set>
#include <memory>
#include <stdexcept>
#include <typeindex>

namespace routing {

    /**
     * @brief Runtime-composable entity that can hold any combination of attributes
     *
     * ComposableEntity replaces the multiple-inheritance model composition
     * (e.g., Client : GeoNode, Consumer, Rendezvous) with a runtime composition
     * model where attributes are stored in a map and accessed dynamically.
     *
     * Usage:
     *   ComposableEntity client(1);
     *   client.addAttribute<GeoNode>(10.0, 20.0);
     *   client.addAttribute<Consumer>(15);
     *
     *   if (client.hasAttribute<Consumer>()) {
     *       auto demand = client.getAttribute<Consumer>().getDemand();
     *   }
     *
     * Note: Uses virtual inheritance from Model to avoid diamond inheritance issues
     * when combined with models::Client, models::Vehicle, or models::Depot.
     */
    class ComposableEntity : public virtual Model {
    public:
        explicit ComposableEntity(unsigned id) {
            setID(id);
        }

        virtual ~ComposableEntity() = default;

        // Prevent copying (attributes contain unique_ptr)
        ComposableEntity(const ComposableEntity&) = delete;
        ComposableEntity& operator=(const ComposableEntity&) = delete;

        // Allow moving
        ComposableEntity(ComposableEntity&&) = default;
        ComposableEntity& operator=(ComposableEntity&&) = default;

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
        std::unique_ptr<ComposableEntity> clone() const {
            auto copy = std::make_unique<ComposableEntity>(getID());
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
     * @brief Composable Client that can be used with legacy code expecting models::Client
     */
    class ComposableClient : public ComposableEntity, public models::Client {
    public:
        explicit ComposableClient(unsigned id)
            : ComposableEntity(id), models::Client(id) {}
    };

    /**
     * @brief Composable Vehicle that can be used with legacy code expecting models::Vehicle
     */
    class ComposableVehicle : public ComposableEntity, public models::Vehicle {
    public:
        explicit ComposableVehicle(unsigned id)
            : ComposableEntity(id), models::Vehicle(id) {}
    };

    /**
     * @brief Composable Depot that can be used with legacy code expecting models::Depot
     */
    class ComposableDepot : public ComposableEntity, public models::Depot {
    public:
        explicit ComposableDepot(unsigned id)
            : ComposableEntity(id), models::Depot(id) {}
    };

} // namespace routing
