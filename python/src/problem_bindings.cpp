// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/optional.h>

#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"
#include "plugins/attributes/ComposableCorePlugin/Entity.hpp"
#include "plugins/attributes/RoutingPlugin/GeoNode.hpp"
#include "plugins/attributes/CapacityPlugin/Consumer.hpp"
#include "plugins/attributes/CapacityPlugin/Stock.hpp"
#include "plugins/attributes/TimeWindowPlugin/Rendezvous.hpp"
#include "plugins/attributes/TimeWindowPlugin/ServiceQuery.hpp"

namespace nb = nanobind;
using namespace routing;

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
            return false;
        }, "Check if entity has a specific attribute");

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
        }, "Service time (if ServiceQuery attribute exists)");

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
        })
        .def_prop_ro("y", [](Depot& d) -> std::optional<double> {
            auto* geo = d.tryGetAttribute<attributes::GeoNode>();
            return geo ? std::optional<double>(geo->getY()) : std::nullopt;
        });

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
        .def("__repr__", [](Problem& p) {
            return "<Problem clients=" + std::to_string(p.getComposableClients().size()) +
                   " vehicles=" + std::to_string(p.getComposableVehicles().size()) + ">";
        });

    // Helper function to add attributes to clients
    m.def("set_client_location", [](Client* c, double x, double y) {
        c->addAttribute<attributes::GeoNode>(x, y);
    }, "Set client location (x, y)");

    m.def("set_client_demand", [](Client* c, int demand) {
        c->addAttribute<attributes::Consumer>(demand);
    }, "Set client demand");

    m.def("set_client_time_window", [](Client* c, double open, double close) {
        c->addAttribute<attributes::Rendezvous>(open, close);
    }, "Set client time window (open, close)");

    m.def("set_client_service_time", [](Client* c, double service) {
        c->addAttribute<attributes::ServiceQuery>(service);
    }, "Set client service time");

    m.def("set_vehicle_capacity", [](Vehicle* v, int capacity) {
        v->addAttribute<attributes::Stock>(capacity);
    }, "Set vehicle capacity");

    m.def("set_depot_location", [](Depot* d, double x, double y) {
        d->addAttribute<attributes::GeoNode>(x, y);
    }, "Set depot location (x, y)");

    m.def("set_depot_time_window", [](Depot* d, double open, double close) {
        d->addAttribute<attributes::Rendezvous>(open, close);
    }, "Set depot time window");
}
