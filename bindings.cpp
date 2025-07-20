/**
 * @file bindings.cpp
 * @author John Korreck
 */

#include <pybind11/pybind11.h>
#include "Engine.h"
#include "Board.h"

namespace py = pybind11;

PYBIND11_MODULE(chessengine, m) {
    m.doc() = "Python bindings for C++ Chess Engine";

    py::class_<Board>(m, "Board")
        .def(py::init<std::string&, std::string&>());

    py::class_<Engine>(m, "Engine")
        .def(py::init<>())
        .def("find_best_move", &Engine::FindBestMove);
}