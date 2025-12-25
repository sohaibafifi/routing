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
#include "plugins/attributes/ProfitPlugin/Profiter.hpp"

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

    // Attribute names for documentation
    attrs.attr("AVAILABLE") = std::vector<std::string>{
        "GeoNode",      // x, y coordinates
        "Consumer",     // demand
        "Stock",        // capacity
        "TimeWindow",   // open, close times
        "ServiceTime",  // service duration
        "Profit"        // profit value
    };
}
