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
        self->m    = Py_None;
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
    static char *kwlist[] = {"row", "col", "goal", NULL};

    // parse args to three ints, third is optional
    // doc: https://docs.python.org/3/c-api/arg.html
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii|i", kwlist,
        &self->row, &self->col, &self->goal))
        return -1;

    return 0;
}

/*
 * Define all map's members.
 */
static PyMemberDef Map_members[] = {
    // private attribute
    {"m", T_OBJECT_EX, offsetof(MapObject, m), READONLY, "map of game"},
    {"row", T_INT, offsetof(MapObject, row), READONLY, "number of rows"},
    {"col", T_INT, offsetof(MapObject, col), READONLY, "number of columns"},
    {"curr", T_INT, offsetof(MapObject, curr), READONLY, "current state of game"},
    {"goal", T_INT, offsetof(MapObject, goal), 0, "goal state of the game"},
    {NULL}	/* Sentinal */
};

static void set_typerr(char *errmsg) {
    PyErr_SetString(PyExc_TypeError, errmsg);
}

static int Map_set_map(MapObject *self, PyObject *m, void *closure) {
    // copy by value; DO NOT COPY BY REF
    // use unsigned index, whereas Py_ssize_t is signed
    PyListObject *tmp, *sublist;
    size_t length;
    if (m == NULL) {
        set_typerr("Cannot delete attribute 'm'");
        return -1;
    } else if (!PyList_Check(m)) {
        set_typerr("Attribute 'm' must be a list");
        return -1;
    } else if (self->m != Py_None) {
        set_typerr("Attribute 'm' has already been set. To set, initialize another instance");
        return -1;
    } else if (self->col == 0 || self->row == 0) {
        PyErr_SetString(PyExc_AttributeError, "Attribute 'row' or 'col' is invalid(0)");
        return -1;
    } else if ((length = (size_t) PyList_GET_SIZE(m)) == 0) {
        set_typerr("Cannot use empty list");
        return -1;
    }

    // assign before expose
    tmp = PyList_New(length);
    if (tmp == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Map.sm: Could not allocate for a new map");
        return -1;
    }
    Py_INCREF(tmp);
    int idx = 0;
    for (int row = 0; row < self->row; row++) {
        if ((sublist = PyList_New(self->col)) == NULL) {
            for (int col = 0; col < self->col; col++) {
                PyList_GetItem(sublist, ((Py_ssize_t) row * self->col + (Py_ssize_t) col));
            }
        } else {
            // TODO: finish
            PyErr_SetString(PyExc_AttributeError, "Map.sm: Could not allocate for a new map");
            return -1;
        }
    }

    for (size_t idx = 0; idx < length; idx++) {
        sublist = PyList_GetItem(tmp, (Py_ssize_t) idx);
        Py_INCREF(sublist);
        for
    }

}

static int Map_set_goal(MapObject *self, PyObject *value, void *closure) {
    if (value == NULL) {
        set_typerr("Cannot delete attribute 'goal'");
        return -1;
    } else if (!PyLong_Check(value)) {
        set_typerr("Attribute 'goal' must be an integer");
        return -1;
    } else {
        self->goal = (int) PyLong_AsLong(value);
        return 0;
    }
}



static PyObject *Map_get_map(MapObject *self, void *closure) {
    // TODO: get a copy of the map
}

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
