// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <sstream>

#include "plugins/attributes/ComposableCorePlugin/Solution.hpp"
#include "plugins/attributes/ComposableCorePlugin/Problem.hpp"

namespace nb = nanobind;
using namespace routing;

void bind_solution(nb::module_& m) {
    // Tour class
    nb::class_<Tour>(m, "Tour")
        .def_prop_ro("cost", &Tour::getCost, "Tour cost")
        .def_prop_ro("num_clients", &Tour::getNbClient, "Number of clients in tour")
        .def("get_client", &Tour::getClient, nb::rv_policy::reference,
             "Get client at position")
        .def("get_client_ids", [](Tour& t) {
            std::vector<int> ids;
            for (unsigned long i = 0; i < t.getNbClient(); ++i) {
                auto* c = t.getClient(i);
                if (c) ids.push_back(c->getID());
            }
            return ids;
        }, "Get list of client IDs in visit order")
        .def("__repr__", [](Tour& t) {
            std::string repr = "<Tour clients=[";
            for (unsigned long i = 0; i < t.getNbClient(); ++i) {
                auto* c = t.getClient(i);
                if (c) {
                    if (i > 0) repr += ", ";
                    repr += std::to_string(c->getID());
                }
            }
            repr += "] cost=" + std::to_string(t.getCost()) + ">";
            return repr;
        })
        .def("__len__", &Tour::getNbClient)
        .def("__getitem__", [](Tour& t, size_t i) -> models::Client* {
            if (i >= t.getNbClient()) {
                throw std::out_of_range("Tour index out of range");
            }
            return t.getClient(i);
        }, nb::rv_policy::reference);

    // Solution class
    nb::class_<Solution>(m, "Solution")
        .def(nb::init<Problem*>(), "Create a new solution for a problem")
        .def_prop_ro("cost", &Solution::getCost, "Total solution cost")
        .def_prop_ro("num_tours", &Solution::getNbTour, "Number of tours")
        .def("get_tour", &Solution::getTour, nb::rv_policy::reference,
             "Get tour by index")
        .def("get_tours", [](Solution& s) {
            std::vector<Tour*> tours;
            for (unsigned long i = 0; i < s.getNbTour(); ++i) {
                tours.push_back(s.getTour(i));
            }
            return tours;
        }, nb::rv_policy::reference, "Get all tours")
        .def_prop_ro("unserved", [](Solution& s) {
            std::vector<int> ids;
            for (auto* c : s.notserved) {
                if (c) ids.push_back(c->getID());
            }
            return ids;
        }, "List of unserved client IDs")
        .def("update", &Solution::update, "Recalculate solution cost")
        .def("print", [](Solution& s) {
            std::ostringstream oss;
            s.print(oss);
            return oss.str();
        }, "Get string representation of solution")
        .def("clone", &Solution::clone, nb::rv_policy::take_ownership,
             "Create a copy of this solution")
        .def("__repr__", [](Solution& s) {
            return "<Solution tours=" + std::to_string(s.getNbTour()) +
                   " cost=" + std::to_string(s.getCost()) +
                   " unserved=" + std::to_string(s.notserved.size()) + ">";
        })
        .def("__len__", &Solution::getNbTour)
        .def("__getitem__", [](Solution& s, size_t i) -> Tour* {
            if (i >= s.getNbTour()) {
                throw std::out_of_range("Solution index out of range");
            }
            return s.getTour(i);
        }, nb::rv_policy::reference);

    // Solution utilities
    m.def("create_solution", [](Problem* p) {
        return new Solution(p);
    }, nb::rv_policy::take_ownership, "Create a new empty solution for a problem");
}
