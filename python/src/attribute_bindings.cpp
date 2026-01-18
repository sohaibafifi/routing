// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

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
using namespace routing::attributes;

void bind_attributes(nb::module_& m) {
    // Create attributes submodule
    auto attrs = m.def_submodule("attributes", "VRP attribute types");

    // GeoNode - coordinates
    nb::class_<GeoNode>(attrs, "GeoNode")
        .def(nb::init<double, double>(), nb::arg("x"), nb::arg("y"),
             "Create a GeoNode with x, y coordinates")
        .def_prop_ro("x", &GeoNode::getX, "X coordinate")
        .def_prop_ro("y", &GeoNode::getY, "Y coordinate")
        .def("__repr__", [](GeoNode& g) {
            return "<GeoNode x=" + std::to_string(g.getX()) +
                   " y=" + std::to_string(g.getY()) + ">";
        });

    // Consumer - demand
    nb::class_<Consumer>(attrs, "Consumer")
        .def(nb::init<int>(), nb::arg("demand"),
             "Create a Consumer with demand")
        .def_prop_ro("demand", &Consumer::getDemand, "Demand quantity")
        .def("__repr__", [](Consumer& c) {
            return "<Consumer demand=" + std::to_string(c.getDemand()) + ">";
        });

    // Stock - vehicle capacity
    nb::class_<Stock>(attrs, "Stock")
        .def(nb::init<double>(), nb::arg("capacity"),
             "Create a Stock with capacity")
        .def_prop_ro("capacity", [](Stock& s) { return static_cast<int>(s.getCapacity()); }, "Capacity")
        .def("__repr__", [](Stock& s) {
            return "<Stock capacity=" + std::to_string(s.getCapacity()) + ">";
        });

    // Rendezvous - time window
    nb::class_<Rendezvous>(attrs, "TimeWindow")
        .def(nb::init<double, double>(), nb::arg("open"), nb::arg("close"),
             "Create a TimeWindow with open and close times")
        .def_prop_ro("open", &Rendezvous::getTwOpen, "Window open time")
        .def_prop_ro("close", &Rendezvous::getTwClose, "Window close time")
        .def("__repr__", [](Rendezvous& r) {
            return "<TimeWindow open=" + std::to_string(r.getTwOpen()) +
                   " close=" + std::to_string(r.getTwClose()) + ">";
        });

    // ServiceQuery - service time
    nb::class_<ServiceQuery>(attrs, "ServiceTime")
        .def(nb::init<double>(), nb::arg("service"),
             "Create a ServiceTime")
        .def_prop_ro("service", &ServiceQuery::getService, "Service duration")
        .def("__repr__", [](ServiceQuery& s) {
            return "<ServiceTime service=" + std::to_string(s.getService()) + ">";
        });

    // Profiter - profit (for TOP problems)
    nb::class_<Profiter>(attrs, "Profit")
        .def(nb::init<double>(), nb::arg("profit"),
             "Create a Profit attribute")
        .def_prop_ro("profit", &Profiter::getProfit, "Profit value")
        .def("__repr__", [](Profiter& p) {
            return "<Profit value=" + std::to_string(p.getProfit()) + ">";
        });

    // Pickup - pickup demand (for P&D problems)
    nb::class_<Pickup>(attrs, "Pickup")
        .def(nb::init<int>(), nb::arg("demand"),
             "Create a Pickup attribute")
        .def_prop_ro("pickup", &Pickup::getPickup, "Pickup demand quantity")
        .def("__repr__", [](Pickup& p) {
            return "<Pickup demand=" + std::to_string(p.getPickup()) + ">";
        });

    // Delivery - delivery demand (for P&D problems)
    nb::class_<Delivery>(attrs, "Delivery")
        .def(nb::init<int>(), nb::arg("demand"),
             "Create a Delivery attribute")
        .def_prop_ro("delivery", &Delivery::getDelivery, "Delivery demand quantity")
        .def("__repr__", [](Delivery& d) {
            return "<Delivery demand=" + std::to_string(d.getDelivery()) + ">";
        });

    // SoftTimeWindows - soft TW penalties (for CVRPSTW problems)
    nb::class_<SoftTimeWindows>(attrs, "SoftTimeWindows")
        .def(nb::init<double, double>(),
             nb::arg("wait_penalty") = 1.0, nb::arg("delay_penalty") = 1.0,
             "Create soft time window configuration")
        .def_prop_ro("wait_penalty", &SoftTimeWindows::getWaitPenalty,
                     "Penalty for early arrival")
        .def_prop_ro("delay_penalty", &SoftTimeWindows::getDelayPenalty,
                     "Penalty for late arrival")
        .def("__repr__", [](SoftTimeWindows& s) {
            return "<SoftTimeWindows wait_penalty=" + std::to_string(s.getWaitPenalty()) +
                   " delay_penalty=" + std::to_string(s.getDelayPenalty()) + ">";
        });

    // Synced - temporal synchronization (for VRPTWTD problems)
    nb::class_<Synced>(attrs, "Synced")
        .def(nb::init<>(), "Create a Synced attribute")
        .def("add_brother", &Synced::addBrother, nb::arg("brother_id"), nb::arg("delta"),
             "Add a synchronization relationship")
        .def("get_brothers_count", &Synced::getBrothersCount,
             "Get number of sync relationships")
        .def("get_brother_id", &Synced::getBrotherId, nb::arg("index"),
             "Get brother ID at index")
        .def("get_delta", &Synced::getDelta, nb::arg("index"),
             "Get time delta at index")
        .def("__repr__", [](Synced& s) {
            return "<Synced brothers=" + std::to_string(s.getBrothersCount()) + ">";
        });

    // Attribute names for documentation
    attrs.attr("AVAILABLE") = std::vector<std::string>{
        "GeoNode",          // x, y coordinates
        "Consumer",         // demand
        "Stock",            // capacity
        "TimeWindow",       // open, close times
        "ServiceTime",      // service duration
        "Profit",           // profit value
        "Pickup",           // pickup demand
        "Delivery",         // delivery demand
        "SoftTimeWindows",  // soft TW penalties
        "Synced"            // temporal sync
    };
}
