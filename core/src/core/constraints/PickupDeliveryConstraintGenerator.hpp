// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once

#include "core/data/IConstraintGenerator.hpp"
#include "core/data/ComposableProblem.hpp"
#include "core/data/AttributeRegistry.hpp"
#include "core/data/attributes/Pickup.hpp"
#include "core/data/attributes/Delivery.hpp"
#include "core/data/attributes/Stock.hpp"

namespace routing {
namespace constraints {

    /**
     * @brief Pickup and Delivery constraint generator
     *
     * Extends capacity constraints for pickup-delivery scenarios where:
     * - Pickup adds goods to the vehicle (increases load)
     * - Delivery removes goods from the vehicle (decreases load)
     * - Net demand = pickup - delivery
     *
     * This generator conflicts with CapacityConstraintGenerator since it
     * provides alternative capacity handling for P&D problems.
     *
     * Requires: Pickup, Delivery, and Stock attributes
     */
    class PickupDeliveryConstraintGenerator : public IConstraintGenerator {
    public:
        std::string name() const override {
            return "PickupDeliveryConstraintGenerator";
        }

        std::vector<AttributeTypeId> requiredAttributes() const override {
            return {
                std::type_index(typeid(attributes::Pickup)),
                std::type_index(typeid(attributes::Delivery)),
                std::type_index(typeid(attributes::Stock))
            };
        }

        std::vector<AttributeTypeId> conflictingAttributes() const override {
            // This replaces standard capacity constraints for P&D problems
            return {};  // No conflicts - P&D clients still need capacity handling
        }

        int priority() const override {
            return 55;  // After routing (10), close to capacity (50)
        }

#ifdef CPLEX_FOUND
        void addVariables(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto vehicles = problem.getComposableVehicles();
            size_t n = clients.size();

            if (vehicles.empty()) return;
            auto* stock = vehicles[0]->tryGetAttribute<attributes::Stock>();
            if (!stock) return;

            Capacity capacity = stock->getCapacity();

            consumption_.clear();

            // Consumption variable for depot (always 0)
            consumption_.push_back(IloNumVar(problem.env, 0, 0, "q_0"));
            problem.model.add(consumption_.back());

            // Consumption variables for each client
            // In P&D, the consumption represents cumulative load after visiting
            for (size_t i = 0; i < n; ++i) {
                auto* client = clients[i];
                auto* pickup = client->tryGetAttribute<attributes::Pickup>();
                auto* delivery = client->tryGetAttribute<attributes::Delivery>();

                PickupDemand pickupAmount = pickup ? pickup->getPickup() : 0;
                DeliveryDemand deliveryAmount = delivery ? delivery->getDelivery() : 0;

                // Net demand: pickup adds, delivery subtracts
                int netDemand = pickupAmount - deliveryAmount;

                std::string varName = "q_" + std::to_string(i + 1);

                // Load can range from 0 to capacity
                consumption_.push_back(IloNumVar(problem.env, 0, capacity, varName.c_str()));
                problem.model.add(consumption_.back());
            }
        }

        void addConstraints(ComposableProblem& problem) override {
            auto clients = problem.getComposableClients();
            auto vehicles = problem.getComposableVehicles();
            size_t n = clients.size();
            size_t m = vehicles.size();

            if (vehicles.empty()) return;
            auto* stock = vehicles[0]->tryGetAttribute<attributes::Stock>();
            if (!stock) return;

            Capacity capacity = stock->getCapacity();

            // Vehicle capacity per route constraint (based on net pickup-delivery)
            for (size_t k = 0; k < m; ++k) {
                auto* vehicleStock = vehicles[k]->tryGetAttribute<attributes::Stock>();
                if (!vehicleStock) continue;

                // Maximum load at any point in route must not exceed capacity
                // This is implicitly handled by the consumption variable bounds
            }

            // MTZ-style load propagation constraints for pickup-delivery
            for (size_t i = 1; i <= n; ++i) {
                for (size_t j = 1; j <= n; ++j) {
                    if (i == j) continue;

                    auto* clientJ = clients[j - 1];
                    auto* pickupJ = clientJ->tryGetAttribute<attributes::Pickup>();
                    auto* deliveryJ = clientJ->tryGetAttribute<attributes::Delivery>();

                    PickupDemand pickupAmount = pickupJ ? pickupJ->getPickup() : 0;
                    DeliveryDemand deliveryAmount = deliveryJ ? deliveryJ->getDelivery() : 0;

                    // Net demand at j: positive = pickup, negative = delivery
                    int netDemandJ = pickupAmount - deliveryAmount;

                    // If arc (i,j) is used, load at j = load at i + net demand at j
                    // consumption[i] + netDemandJ <= consumption[j] + capacity * (1 - arcs[i][j])
                    problem.model.add(
                        consumption_[i] + netDemandJ
                        <= consumption_[j] + capacity * (1 - problem.arcs[i][j])
                    );
                }
            }

            // Constraints for arcs from depot
            for (size_t j = 1; j <= n; ++j) {
                auto* clientJ = clients[j - 1];
                auto* pickupJ = clientJ->tryGetAttribute<attributes::Pickup>();
                auto* deliveryJ = clientJ->tryGetAttribute<attributes::Delivery>();

                PickupDemand pickupAmount = pickupJ ? pickupJ->getPickup() : 0;
                DeliveryDemand deliveryAmount = deliveryJ ? deliveryJ->getDelivery() : 0;
                int netDemandJ = pickupAmount - deliveryAmount;

                // If depot->j is used, load at j = net demand at j
                problem.model.add(
                    netDemandJ * problem.arcs[0][j] <= consumption_[j]
                );
            }

            // Ensure load returns to depot empty (delivery-only) or
            // allows returning with collected pickups
            for (size_t i = 1; i <= n; ++i) {
                // Load when returning to depot must respect capacity
                problem.model.add(
                    consumption_[i] <= capacity
                );
            }
        }

    private:
        std::vector<IloNumVar> consumption_;
#endif
    };

    // Auto-register this generator
    ROUTING_REGISTER_GENERATOR(PickupDeliveryConstraintGenerator);

} // namespace constraints
} // namespace routing
