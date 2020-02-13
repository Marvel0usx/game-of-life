/* Game-of-live map implementation */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/*
 * Documentation for map.
 */
PyDoc_STRVAR(map_mod_doc, "This module implements the map of 2D version 
    of the 'Game of Life' and a generator of the map, which returns the 
    current state and updates the state of the map when called.\nImport: 
    from map import Map, MapGen");

PyDoc_STRVAR(map_obj_doc, "Usage: map.Map()");

/*
 * Custom PyObject for Map
 */
typedef struct {
    PyObject_HEAD
    // Type specifications
    int _row;
    int _col;
    int _curr;
    int _goal;
    // TODO: _map int[][]

} MapObject;

/*
 * Configurations for Map
 */
static PyTypeObject MapType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "map.Map",
    .tp_doc  = map_obj_doc,
    .tp_basicsize = sizeof(MapObject),
    .tp_itemsize  = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
};

/*
 * Configurations for map
 */
static PyModuleDef mapmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "map",
    .m_doc = map_mod_doc,
    .m_size = -1,
};

/*
 * Initialization of module
 */
PyMODINIT_FUNC PyInit_map(void) {
    PyObject *map;
    if (PyType_Ready(&MapType) < 0)
        return NULL;

    map = PyModule_Create(&mapmodule);
    if (map == NULL)
        return NULL;

    Py_INCREF(&MapType);
    if (PyModule_AddObject(map, "Map", (PyObject *) &MapType) < 0) {
        Py_DECREF(&MapType);
        Py_DECREF(map);
        return NULL;
    }

    return map;
}