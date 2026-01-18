// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/optional.h>

#include <stdexcept>

#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Entity.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"
#include "plugins/attributes/TimeWindowPlugin/SoftTimeWindows.hpp"
#include "plugins/attributes/ProfitPlugin/Profiter.hpp"
#include "plugins/attributes/PickupDeliveryPlugin/Pickup.hpp"
#include "plugins/attributes/PickupDeliveryPlugin/Delivery.hpp"
#include "plugins/attributes/SyncPlugin/Synced.hpp"

namespace nb = nanobind;
using namespace routing;

namespace {

void enable_attribute_by_name(Problem& p, const std::string& name) {
    if (name == "GeoNode") {
        p.enableAttribute<attributes::GeoNode>();
        return;
    }
    if (name == "Consumer") {
        p.enableAttribute<attributes::Consumer>();
        return;
    }
    if (name == "Stock") {
        p.enableAttribute<attributes::Stock>();
        return;
    }
    if (name == "Rendezvous") {
        p.enableAttribute<attributes::Rendezvous>();
        return;
    }
    if (name == "ServiceQuery") {
        p.enableAttribute<attributes::ServiceQuery>();
        return;
    }
    if (name == "Profiter") {
        p.enableAttribute<attributes::Profiter>();
        return;
    }
    if (name == "Pickup") {
        p.enableAttribute<attributes::Pickup>();
        return;
    }
    if (name == "Delivery") {
        p.enableAttribute<attributes::Delivery>();
        return;
    }
    if (name == "SoftTimeWindows") {
        p.enableAttribute<attributes::SoftTimeWindows>();
        return;
    }
    if (name == "Synced") {
        p.enableAttribute<attributes::Synced>();
        return;
    }

    throw std::runtime_error("Unknown attribute: " + name);
}

} // namespace

void bind_problem(nb::module_& m) {
    // Entity base class (exposed for type hints)
    nb::class_<Entity>(m, "Entity")
        .def("get_id", &Entity::getID, "Get entity ID")
        .def("has_attribute", [](Entity& e, const std::string& name) {
            // Check common attributes by name
            if (name == "GeoNode") return e.hasAttribute<attributes::GeoNode>();
            if (name == "Consumer") return e.hasAttribute<attributes::Consumer>();
            if (name == "Stock") return e.hasAttribute<attributes::Stock>();
            if (name == "Rendezvous") return e.hasAttribute<attributes::Rendezvous>();
            if (name == "ServiceQuery") return e.hasAttribute<attributes::ServiceQuery>();
            if (name == "Profiter") return e.hasAttribute<attributes::Profiter>();
            if (name == "Pickup") return e.hasAttribute<attributes::Pickup>();
            if (name == "Delivery") return e.hasAttribute<attributes::Delivery>();
            if (name == "SoftTimeWindows") return e.hasAttribute<attributes::SoftTimeWindows>();
            if (name == "Synced") return e.hasAttribute<attributes::Synced>();
            return false;
        }, "Check if entity has a specific attribute")
        .def("add_attribute", [](Entity& e, const std::string& name, nb::args args, nb::kwargs kwargs) {
            // Generic attribute addition by name
            if (name == "GeoNode") {
                if (args.size() >= 2) {
                    double x = nb::cast<double>(args[0]);
                    double y = nb::cast<double>(args[1]);
                    e.addAttribute<attributes::GeoNode>(x, y);
                    return;
                }
            }
            if (name == "Consumer") {
                if (args.size() >= 1) {
                    int demand = nb::cast<int>(args[0]);
                    e.addAttribute<attributes::Consumer>(demand);
                    return;
                }
            }
            if (name == "Stock") {
                if (args.size() >= 1) {
                    int capacity = nb::cast<int>(args[0]);
                    e.addAttribute<attributes::Stock>(capacity);
                    return;
                }
            }
            if (name == "Rendezvous") {
                if (args.size() >= 2) {
                    double open = nb::cast<double>(args[0]);
                    double close = nb::cast<double>(args[1]);
                    e.addAttribute<attributes::Rendezvous>(open, close);
                    return;
                }
            }
            if (name == "ServiceQuery") {
                if (args.size() >= 1) {
                    double service = nb::cast<double>(args[0]);
                    e.addAttribute<attributes::ServiceQuery>(service);
                    return;
                }
            }
            if (name == "Profiter") {
                if (args.size() >= 1) {
                    double profit = nb::cast<double>(args[0]);
                    e.addAttribute<attributes::Profiter>(profit);
                    return;
                }
            }
            if (name == "Pickup") {
                if (args.size() >= 1) {
                    int pickup = nb::cast<int>(args[0]);
                    e.addAttribute<attributes::Pickup>(pickup);
                    return;
                }
            }
            if (name == "Delivery") {
                if (args.size() >= 1) {
                    int delivery = nb::cast<int>(args[0]);
                    e.addAttribute<attributes::Delivery>(delivery);
                    return;
                }
            }
            if (name == "SoftTimeWindows") {
                if (args.size() >= 2) {
                    double wait_penalty = nb::cast<double>(args[0]);
                    double delay_penalty = nb::cast<double>(args[1]);
                    e.addAttribute<attributes::SoftTimeWindows>(wait_penalty, delay_penalty);
                    return;
                }
            }
            if (name == "Synced") {
                e.addAttribute<attributes::Synced>();
                return;
            }

            throw std::runtime_error("Unknown attribute '" + name + "' or invalid parameters");
        }, "Add attribute by name with parameters. Usage: entity.add_attribute('GeoNode', 10.0, 20.0)");

    // Client class
    nb::class_<Client, Entity>(m, "Client")
        .def("get_id", &Client::getID)
        .def_prop_ro("x", [](Client& c) -> std::optional<double> {
            auto* geo = c.tryGetAttribute<attributes::GeoNode>();
            return geo ? std::optional<double>(geo->getX()) : std::nullopt;
        }, "X coordinate (if GeoNode attribute exists)")
        .def_prop_ro("y", [](Client& c) -> std::optional<double> {
            auto* geo = c.tryGetAttribute<attributes::GeoNode>();
            return geo ? std::optional<double>(geo->getY()) : std::nullopt;
        }, "Y coordinate (if GeoNode attribute exists)")
        .def_prop_ro("demand", [](Client& c) -> std::optional<int> {
            auto* consumer = c.tryGetAttribute<attributes::Consumer>();
            return consumer ? std::optional<int>(consumer->getDemand()) : std::nullopt;
        }, "Demand (if Consumer attribute exists)")
        .def_prop_ro("tw_open", [](Client& c) -> std::optional<double> {
            auto* tw = c.tryGetAttribute<attributes::Rendezvous>();
            return tw ? std::optional<double>(tw->getTwOpen()) : std::nullopt;
        }, "Time window open (if Rendezvous attribute exists)")
        .def_prop_ro("tw_close", [](Client& c) -> std::optional<double> {
            auto* tw = c.tryGetAttribute<attributes::Rendezvous>();
            return tw ? std::optional<double>(tw->getTwClose()) : std::nullopt;
        }, "Time window close (if Rendezvous attribute exists)")
        .def_prop_ro("service_time", [](Client& c) -> std::optional<double> {
            auto* svc = c.tryGetAttribute<attributes::ServiceQuery>();
            return svc ? std::optional<double>(svc->getService()) : std::nullopt;
        }, "Service time (if ServiceQuery attribute exists)")
        .def_prop_ro("pickup", [](Client& c) -> std::optional<int> {
            auto* pickup = c.tryGetAttribute<attributes::Pickup>();
            return pickup ? std::optional<int>(pickup->getPickup()) : std::nullopt;
        }, "Pickup demand (if Pickup attribute exists)")
        .def_prop_ro("delivery", [](Client& c) -> std::optional<int> {
            auto* delivery = c.tryGetAttribute<attributes::Delivery>();
            return delivery ? std::optional<int>(delivery->getDelivery()) : std::nullopt;
        }, "Delivery demand (if Delivery attribute exists)")
        .def_prop_ro("profit", [](Client& c) -> std::optional<double> {
            auto* profit = c.tryGetAttribute<attributes::Profiter>();
            return profit ? std::optional<double>(profit->getProfit()) : std::nullopt;
        }, "Profit value (if Profiter attribute exists)");

    // Vehicle class
    nb::class_<Vehicle, Entity>(m, "Vehicle")
        .def("get_id", &Vehicle::getID)
        .def_prop_ro("capacity", [](Vehicle& v) -> std::optional<int> {
            auto* stock = v.tryGetAttribute<attributes::Stock>();
            return stock ? std::optional<int>(stock->getCapacity()) : std::nullopt;
        }, "Capacity (if Stock attribute exists)");

    // Depot class
    nb::class_<Depot, Entity>(m, "Depot")
        .def("get_id", &Depot::getID)
        .def_prop_ro("x", [](Depot& d) -> std::optional<double> {
            auto* geo = d.tryGetAttribute<attributes::GeoNode>();
            return geo ? std::optional<double>(geo->getX()) : std::nullopt;
        }, "X coordinate (if GeoNode attribute exists)")
        .def_prop_ro("y", [](Depot& d) -> std::optional<double> {
            auto* geo = d.tryGetAttribute<attributes::GeoNode>();
            return geo ? std::optional<double>(geo->getY()) : std::nullopt;
        }, "Y coordinate (if GeoNode attribute exists)")
        .def_prop_ro("tw_open", [](Depot& d) -> std::optional<double> {
            auto* tw = d.tryGetAttribute<attributes::Rendezvous>();
            return tw ? std::optional<double>(tw->getTwOpen()) : std::nullopt;
        }, "Time window open (if Rendezvous attribute exists)")
        .def_prop_ro("tw_close", [](Depot& d) -> std::optional<double> {
            auto* tw = d.tryGetAttribute<attributes::Rendezvous>();
            return tw ? std::optional<double>(tw->getTwClose()) : std::nullopt;
        }, "Time window close (if Rendezvous attribute exists)");

    // Problem class
    nb::class_<Problem>(m, "Problem")
        .def(nb::init<>(), "Create a new empty problem")
        .def("add_client", [](Problem& p, int id) {
            return p.addClient(id);
        }, nb::rv_policy::reference, "Add a client with given ID")
        .def("add_vehicle", [](Problem& p, int id) {
            return p.addVehicle(id);
        }, nb::rv_policy::reference, "Add a vehicle with given ID")
        .def("add_depot", [](Problem& p, int id) {
            return p.addDepot(id);
        }, nb::rv_policy::reference, "Add a depot with given ID")
        .def("get_client", [](Problem& p, int id) {
            auto clients = p.getComposableClients();
            for (auto* c : clients) {
                if (c->getID() == id) return c;
            }
            return static_cast<Client*>(nullptr);
        }, nb::rv_policy::reference, "Get client by ID")
        .def("get_depot", [](Problem& p, int id) {
            auto depots = p.getComposableDepots();
            for (auto* d : depots) {
                if (d->getID() == id) return d;
            }
            return static_cast<Depot*>(nullptr);
        }, nb::rv_policy::reference, "Get depot by ID")
        .def("get_clients", [](Problem& p) {
            return p.getComposableClients();
        }, nb::rv_policy::reference, "Get all clients")
        .def("get_vehicles", [](Problem& p) {
            return p.getComposableVehicles();
        }, nb::rv_policy::reference, "Get all vehicles")
        .def("get_depots", [](Problem& p) {
            return p.getComposableDepots();
        }, nb::rv_policy::reference, "Get all depots")
        .def_prop_ro("num_clients", [](Problem& p) {
            return p.getComposableClients().size();
        }, "Number of clients")
        .def_prop_ro("num_vehicles", [](Problem& p) {
            return p.getComposableVehicles().size();
        }, "Number of vehicles")
        .def_prop_ro("num_depots", [](Problem& p) {
            return p.getComposableDepots().size();
        }, "Number of depots")
        .def_prop_ro("total_demand", [](Problem& p) {
            int total = 0;
            for (auto* client : p.getComposableClients()) {
                auto* consumer = client->tryGetAttribute<attributes::Consumer>();
                if (consumer) {
                    total += consumer->getDemand();
                }
            }
            return total;
        }, "Total demand across all clients")
        .def_prop_ro("total_capacity", [](Problem& p) {
            int total = 0;
            for (auto* vehicle : p.getComposableVehicles()) {
                auto* stock = vehicle->tryGetAttribute<attributes::Stock>();
                if (stock) {
                    total += static_cast<int>(stock->getCapacity());
                }
            }
            return total;
        }, "Total capacity across all vehicles")
        .def("get_distance", [](Problem& p, int from_id, int to_id) {
            auto clients = p.getComposableClients();
            auto depots = p.getComposableDepots();

            Client* fromClient = nullptr;
            Client* toClient = nullptr;
            Depot* fromDepot = nullptr;
            Depot* toDepot = nullptr;

            // Find entities by ID
            for (auto* c : clients) {
                if (c->getID() == from_id) fromClient = c;
                if (c->getID() == to_id) toClient = c;
            }
            for (auto* d : depots) {
                if (d->getID() == from_id) fromDepot = d;
                if (d->getID() == to_id) toDepot = d;
            }

            // Calculate distance based on entity types
            if (fromClient && toClient) {
                return p.getDistance(*fromClient, *toClient);
            } else if (fromClient && toDepot) {
                return p.getDistance(*fromClient, *toDepot);
            } else if (fromDepot && toClient) {
                return p.getDistance(*fromDepot, *toClient);
            }
            return 0.0;
        }, "Get distance between two nodes by ID")
        .def("enable_attribute", &enable_attribute_by_name, nb::arg("name"),
             "Enable an attribute by name")
        .def("enable_attributes", [](Problem& p, const std::vector<std::string>& names) {
            for (const auto& name : names) {
                enable_attribute_by_name(p, name);
            }
        }, nb::arg("names"), "Enable multiple attributes by name")
        .def("__repr__", [](Problem& p) {
            return "<Problem clients=" + std::to_string(p.getComposableClients().size()) +
                   " vehicles=" + std::to_string(p.getComposableVehicles().size()) + ">";
        });

}
