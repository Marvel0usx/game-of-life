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
    PyObject *m;
    int row;
    int col;
    int curr;
    int goal;
} MapObject;

/* Since we have some data to manage, we at least need to have
   deallocation. */
static void Map_dealloc(MapObject *self) {
    PyObject *tmp;
    for (Py_ssize_t i = 0; i < self->row; i++) {
        tmp = PyList_GetItem(self->m, i);
        PyList_SetItem(self->m, i, NULL);
        Py_XDECREF(tmp);
    }
    tmp = self->m;
    self->m = NULL;
    Py_XDECREF(tmp);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject *
Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    MapObject *self;
    /* allocate memory for self, and since we did not fill the tp_alloc slot,
       PyType_Ready() is used as default by object */
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

static int
Map_init(MapObject *self, PyObject *args, PyObject *kwds) {
    /* notice that in the previous function we have
       new-ed self hence we can use it here */
    static char *kwlist[] = {"row", "col", "goal", NULL}; /* null-terminated */

    /* parse args to three ints, third is optional
       doc: https://docs.python.org/3/c-api/arg.html */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii|i", kwlist,
        &self->row, &self->col, &self->goal))
        return -1;

    return 0;
}

/*
 * Define all map's members.
 */
static PyMemberDef Map_members[] = {
    {"m", T_OBJECT_EX, offsetof(MapObject, m), READONLY, "map of game"},
    {"row", T_INT, offsetof(MapObject, row), READONLY, "number of rows"},
    {"col", T_INT, offsetof(MapObject, col), READONLY, "number of columns"},
    {"curr", T_INT, offsetof(MapObject, curr), READONLY, "current state of game"},
    {"goal", T_INT, offsetof(MapObject, goal), READONLY, "goal state of the game"},
    {NULL}	/* Sentinal */
};

static int Map_set_map(MapObject *self, PyObject *m, void *closure) {
    /* copy by value; DO NOT COPY BY REF use 
     * unsigned index, whereas Py_ssize_t is signed 
     */
    Py_ssize_t length, idx;
    	
    /* Error checking before initializing */
    if (m == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete attribute 'm'");
        return -1;
    } else if (!PyList_Check(m)) {
        PyErr_SetString(PyExc_TypeError, "Argument 'm' must be a list");
        return -1;
    } else if (self->m != Py_None) {
        PyErr_SetString(PyExc_TypeError, "Attribute 'm' has already been set. To set, initialize another instance");
        return -1;
    } else if (self->col == 0 || self->row == 0) {
        PyErr_SetString(PyExc_AttributeError, "Attribute 'row' or 'col' is invalid(0)");
        return -1;
    } else if ((length = PyList_Size(m)) == 0) {
        PyErr_SetString(PyExc_ValueError, "Cannot use empty list to initialize attribute 'm'");
        return -1;
    } else if (self->row * self->col < length) {
        PyErr_SetString(PyExc_IndexError, "Number of elements in 'm' excesses bound");
        return -1;
    }

    /* Initiaze Map.m to a list */
    self->m = PyList_New((Py_ssize_t) self->row);
    if (!self->m) {
        PyErr_SetString(PyExc_MemoryError, "Could not allocate for Map.m");
        return -1;
    }

    idx = 0;
    /* Fill in the contents */
    for (Py_ssize_t row = 0; row < self->row; row++) {
        if (PyList_SetItem(self->m, row, PyList_New(self->col)) == -1) {  /* steal ref */
            PyErr_SetString(PyExc_RuntimeError, "Could not append to attribute 'm'");
            return -1;
        }
        for (Py_ssize_t col = 0; col < self->col; col++) {
            PyObject *cell = PyList_GetItem(m, idx);  /* borrowed ref */
            if (!PyLong_Check(cell)) {
                PyErr_SetString(PyExc_TypeError, "Contents of the list should be int");
                Py_DECREF(cell);
                return -1;
            } else if (PyLong_AsLong(cell) != 0 || PyLong_AsLong(cell) != 1) {
                PyErr_SetString(PyExc_ValueError, "The value should only be 0 or 1");
                Py_DECREF(cell);
                return -1;
            }
            if (PyList_Append(PyList_GetItem(self->m, row), cell) == -1) {
                PyErr_SetString(PyExc_RuntimeError, "Could not append to list");
                Py_DECREF(cell);
                return -1;
            }
            idx++;
        }
    }
    return 0;
}

static int Map_set_goal(MapObject *self, PyObject *value, void *closure) {
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete attribute 'goal'");
        return -1;
    } else if (!PyLong_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Attribute 'goal' must be an integer");
        return -1;
    } else {
        self->goal = PyLong_AsLong(value); /* does not create new ref */
        return 0;
    }
}

static PyObject *Map_get_map(MapObject *self, void *closure) {
    if (self->m != Py_None) {
        return PyList_AsTuple(self->m);
    } else {
        return Py_None;
    }
}

static PyObject *
Map_gen(PyObject *self, PyObject *Py_UNUSED(ignore)) {
    /* TODO: generator */
    return PyUnicode_FromString("Dummy");
}

/*
 * Define all Map's methods.
 */
static PyMethodDef Map_methods[] = {
    {"gen", (PyCFunction) Map_gen, METH_NOARGS,
     "Return the generator for this game's map"},
    {"set_goal", (PyCFunction) Map_set_goal, METH_VARARGS,
     "Set the goal state of the map"},
    {"set_map", (PyCFunction) Map_set_map, METH_VARARGS,
     "Set the map"},
    {"get_map", (PyCFunction) Map_get_map, METH_NOARGS,
     "Get a tuple version of the map"},
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
    .tp_flags = Py_TPFLAGS_DEFAULT,                         /* not subtypable */
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = Map_new,
    .tp_init = (initproc) Map_init,
    .tp_dealloc = (destructor) Map_dealloc,                 /* destructor */
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
