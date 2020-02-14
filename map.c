/* Game-of-live map implementation */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

/*
 * Documentation for map.
 */
PyDoc_STRVAR(map_mod_doc, "This module implements the map of 2D version \
    of the 'Game of Life' and a generator of the map, which returns the \
    current state and updates the state of the map when called.\nImport: \
    from map import Map, MapGen");

PyDoc_STRVAR(map_obj_doc, "Usage: map.Map()");

/*
 * Custom PyObject for Map.
 */
typedef struct {
    PyObject_HEAD
    PyObject *m;   // game map
    int row;       // row count
    int col;       // col count
    int curr;      // current state
    int goal;      // goal state
} MapObject;

// Since we have some data to manage, we at least need to have
// deallocation.
static void Map_dealloc(MapObject *self) {
    Py_XDECREF(self->m);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

/*
 * __new__
 */
static PyObject *
Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    MapObject *self;
    // allocate memory for self, and since we did not fill the tp_alloc slot,
    // PyType_Ready() is used as default by object
    self = (MapObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        // TODO: allocate mem, init for List[List[int]]
        // self->m = PyList_From...();
        self->row  = 0;
        self->col  = 0;
        self->curr = 0;
        self->goal = 0;
    }
    return (PyObject *) self;
}

/*
 * __init__
 */
static int
Map_init(MapObject *self, PyObject *args, PyObject *kwds) {
    /* notice that in the previous function we have
       new-ed self hence we can use it here */
    // null-terminated array
    static char *kwlist[] = {"map", "row", "col", "goal", NULL};
    PyObject *m = NULL, *tmp;

    // parse args to one PyObject and three int
    // doc: https://docs.python.org/3/c-api/arg.html
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oiii", kwlist,
        &m, &self->row, &self->col, &self->goal))
        return -1;

    if (m) {
        tmp = self->m;   // always reassign before DECREF
        Py_INCREF(m);
        self->m = m;
        Py_XDECREF(tmp); // decrement ref count, *o can be NULL
    }
    return 0;
}

/*
 * Define all map's members.
 */
static PyMemberDef Map_members[] = {
    {"m", T_OBJECT_EX, offsetof(MapObject, m), 0, "map of game"},
    {"row", T_INT, offsetof(MapObject, row), 0, "number of rows"},
    {"col", T_INT, offsetof(MapObject, col), 0, "number of columns"},
    {"curr", T_INT, offsetof(MapObject, curr), 0, "current state of game"},
    {"goal", T_INT, offsetof(MapObject, goal), 0, "goal state of the game"},
    {NULL}	/* Sentinal */
};

/*
 * Method: Map.set_map
 */
/* TODO
static PyObject *Map_set_map(MapObject *self, TODO add argument: 2d array) {
    if (self->m == NULL) {
	PyErr_SetString(PyExc_AttributeError, "m");
	return NULL;
    }
    return; //TODO set m to a 2d array
}
*/

/*
 * Method: Map.print_map
 */
static PyObject *Map_print_map(MapObject *self, PyObject *Py_UNUSED(ignored)) {
    /* TODO: remoce dummy code
    if (self->m == NULL) {
	PyErr_SetString(PyExc_AttributeError, "m");
	return NULL;
    }
    */
    return PyUnicode_FromFormat("Haha!");
}

/*
 * Method: Map.set_dimesions
 */
/* TODO
static PyObject *Map_set_dimensions(MapObject *self, PyObject *row, PyObject *col) {
    if (self->row == NULL) {
	PyErr_SetString(PyExc_AttributeError, "row");
	return NULL;
    } else if (self->col == NULL) {
	PyErr_SetString(PyExc_AttributeError, "col");
	return NULL;
    } else {
	if ((self->row = PyLong_AsLong(row)) == -1) {
		PyErr_SetString(PyExec_TypeError, "row");
		return NULL;
	}
	if ((self->col = PyLong_AsLong(col)) == -1) {
		PyErr_SetString(PyExec_TypeError, "col");
		return NULL;
	}
    }
    return NULL;
}
*/

/*
 * Define all Map's methods.
 */
static PyMethodDef Map_methods[] = {
    {"print_map", (PyCFunction) Map_print_map, METH_NOARGS, 
     "Return the string representation of map"},
    {NULL}
};

/*
 * Configurations for Map.
 */
static PyTypeObject MapType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "map.Map",
    .tp_doc  = map_obj_doc,
    .tp_basicsize = sizeof(MapObject),
    .tp_itemsize  = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    // not subtypable
    .tp_new = Map_new,
    .tp_dealloc = (destructor) Map_dealloc, // destructor
    .tp_init = (initproc) Map_init,
    .tp_members = Map_members,
    .tp_methods = Map_methods,
};

/*
 * Configurations for map.
 */
static PyModuleDef mapmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "map",
    .m_doc = map_mod_doc,
    .m_size = -1,
};

/*
 * Initialization of module.
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
