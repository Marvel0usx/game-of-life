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
	Py_XDECREF(self->m);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject *
Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    MapObject *self;
    /* allocate memory for self, and since we did not fill the tp_alloc slot,
       PyType_Ready() is used as default by object */
    self = (MapObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
	    self->m = Py_None;
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
    /* parse args to three ints, third is optional
       doc: https://docs.python.org/3/c-api/arg.html */
    if (!PyArg_ParseTuple(args, "ll|l" /* only for ParseTuple*/,
	    &self->row, &self->col, &self->goal))
        return -1;
    return 0;
}

/*
 * Define all map's members.
 */
static PyMemberDef Map_members[] = {
    {"row", T_INT, offsetof(MapObject, row), READONLY, "number of rows"},
    {"col", T_INT, offsetof(MapObject, col), READONLY, "number of columns"},
    {"curr", T_INT, offsetof(MapObject, curr), READONLY, "current state of game"},
    {"goal", T_INT, offsetof(MapObject, goal), 0, "goal state of the game"},
    {NULL}	/* Sentinal */
};

static PyObject *Map_set_map(MapObject *self, PyObject *m, void *closure) {
	Py_XINCREF(m);
    /* copy by value; DO NOT COPY BY REF use
     * unsigned index, whereas Py_ssize_t is signed
     */
    Py_ssize_t length, idx = 0;
	PyObject * cell, *tmp, *pMap = NULL;		/* no new ref */
    /* error checking */
	if (!PyArg_ParseTuple(m, "O!:", &PyList_Type, &pMap /* here pMap receives the actual obj passed, no new reference */)) {
		PyErr_SetString(PyExc_TypeError, "Parameter must be a list.");
		goto error;
    } else if (self->m != Py_None) {
        PyErr_SetString(PyExc_TypeError, "Map has already been set. To set, initialize another instance.");
        goto error;
    } else if (self->col == 0 || self->row == 0) {
        PyErr_SetString(PyExc_AttributeError, "Attribute 'row' or 'col' is invalid(0).");
        goto error;
    } else if ((length = PyList_Size(pMap)) == 0) {
        PyErr_SetString(PyExc_ValueError, "Cannot use empty list to initialize map.");
        goto error;
    } else if (self->row * self->col != length) {
        PyErr_SetString(PyExc_IndexError, "Too many or too few elements.");
		goto error;
    }
    /* initialize map*/
    self->m = PyList_New(self->row);
    for (Py_ssize_t row = 0; row < self->row; row++) {
        tmp = (PyObject *) PyList_New(self->col);
        for (Py_ssize_t col = 0; col < self->col; col++, idx++) {
            if (!PyLong_Check((cell = PyList_GetItem(pMap, idx)) /* borrowed ref */)) {
                PyErr_SetString(PyExc_TypeError, "List elements must be integer valued.");
                goto error2;
            } else if (PyLong_AsLong(cell) != 0 && PyLong_AsLong(cell) != 1) {
                PyErr_SetString(PyExc_TypeError, "List elements must be either 0 or 1.");
                goto error2;
            } else {
                PyList_SetItem(tmp, col, cell);		/* steal reference of tmp */
            }
        }
        PyList_SetItem(self->m, row, tmp);
		/* no need to DECREF(tmp) since it causes self->m[i] to be garbage-collected */
    }
	Py_DECREF(m);
	Py_RETURN_NONE;

error:
	Py_XDECREF(m);
	return NULL;

error2:
	self->m = Py_None;
	Py_XDECREF(m);
	Py_XDECREF(tmp);
	return NULL;
}  

static PyObject *Map_get_map(MapObject *self, void *closure) {
    if (self->m != Py_None) {
        return PyList_AsTuple(self->m);
    } else {
        return Py_None;
    }
}

static PyObject *Map_gen(PyObject *self, PyObject *Py_UNUSED(ignore)) {
    /* TODO: generator */
    return PyUnicode_FromString("Dummy");
}

/*
 * Define all Map's methods.
 */
static PyMethodDef Map_methods[] = {
    {"gen", (PyCFunction) Map_gen, METH_NOARGS,
     "Return the generator for this game's map"},
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
