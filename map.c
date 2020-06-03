/* Game-of-live map implementation */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include "life_helpers.h"

/*
 * Documentation for map.
 */
PyDoc_STRVAR(map_mod_doc, "This module implements the map of 2D version \
    of the 'Game of Life' and a generator of the map, which returns the \
    current state and updates the state of the map when called.\
	Import: from map import Map, MapIter");

PyDoc_STRVAR(map_obj_doc, "Usage: map.Map(row: int, col: int)");

/*
 * Custom PyObject for Map.
 */
typedef struct {
    PyObject_HEAD
    PyObject *m;
    int row;
    int col;
} MapObject;

/* Since we have some data to manage, we at least need to have
   deallocation. */
static void Map_dealloc(MapObject *self) {
	Py_XDECREF(self->m);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject *Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    MapObject *self;
    /* allocate memory for self, and since we did not fill the tp_alloc slot,
       PyType_Ready() is used as default by object */
    self = (MapObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
	    self->m = Py_None;
        self->row  = 0;
        self->col  = 0;
    } 
    return (PyObject *) self;
}

static int Map_init(MapObject *self, PyObject *args, PyObject *kwds) {
    /* notice that in the previous function we have
       new-ed self hence we can use it here */
    /* parse args to three ints, third is optional
       doc: https://docs.python.org/3/c-api/arg.html */
    if (!PyArg_ParseTuple(args, "ll" /* only for ParseTuple*/,
	    &self->row, &self->col))
        return -1;
    return 0;
}

/*
 * Define all map's members.
 */
static PyMemberDef Map_members[] = {
    {"row", T_INT, offsetof(MapObject, row), READONLY, "number of rows"},
    {"col", T_INT, offsetof(MapObject, col), READONLY, "number of columns"},
    {NULL}	/* Sentinal */
};

static PyObject *Map_set_map(MapObject *self, PyObject *m, void *closure) {
	Py_XINCREF(m);
    /* copy by value; DO NOT COPY BY REF use
     * unsigned index, whereas Py_ssize_t is signed
     */
    Py_ssize_t length = 0, idx = 0;
	PyObject * cell, *tmp, *pMap;		/* no new ref */
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

/* TODO: Return a new iterator. */
// static PyObject *Map_get_iter(MapObject *self, void *closure);

/*
 * Define all Map's methods.
 */
static PyMethodDef Map_methods[] = {
    {"set_map", (PyCFunction) Map_set_map, METH_VARARGS,
     "Set the content of the map."},
    {"get_map", (PyCFunction) Map_get_map, METH_NOARGS,
     "Get a tuple version of the map."},
	/*{"get_iter", (PyCFunction) Map_get_iter, METH_NOARGS,
	 "Get a new iterator of the map object."},*/
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

/* Map Iterator Class */
typedef struct {
	PyObject_HEAD
	MapObject *m;
	int **buf;
	int curr;
	int goal;
} MapIterObject;

static void MapIter_dealloc(MapIterObject *self) {
	for (int **tmp = self->buf; tmp - self->buf < self->m->row; tmp++) {
		free(*tmp);
	}
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *MapIter_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	MapIterObject *self;
	self = (MapIterObject *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->curr = 0;
		self->goal = 0;
		self->buf = NULL;
		self->m = malloc(sizeof(MapObject *));
	}
	return (PyObject *)self;
}

static int MapIter_init(MapIterObject *self, PyObject *args, PyObject *kwds) {
	if (!PyArg_ParseTuple(args, "Ol" /* only for ParseTuple*/,
		&self->m, &self->goal))
		return -1;
	return 0;
}

static PyMemberDef MapIter_members[] = {
	{"curr", T_INT, offsetof(MapIterObject, curr), READONLY, "current state"},
	{"goal", T_INT, offsetof(MapIterObject, goal), 0, "goal state"},
	{NULL}	/* Sentinal */
};

/* TODO: __iter__ and __next__*/
// set tp_iternext to this function
// Python interpreter checks on whether the tp_iter is provided
// if no tp_iter is provided, it checks whether the type is sequence
// in this case, the MapIter is not of a sequence type. Therefore,
// fail to provide tp_iter cause "object is not iterable" exception.
// tp_iter should return another object that passes PyIter_Check.
// No new reference created.
PyObject *MapIter_GetIter(PyObject *self) {
	return self;
}

// set tp_iternext to this function
// tp_iternext should take an iterator, which would be the iterable
// returned by tp_iter. Return of this function creates new reference.
// Return NULL with no exception set when finished
PyObject *MapIter_GetNext(MapIterObject *self) {
	if (self->curr < self->goal) {
        update_map(self->map, self->m->rows, self->m->cols);
        return self;
    } else {
        return NULL;
    }
}

PyObject *MapIter_GetMap(MapIterObject *self) {
    
}

static PyMethodDef MapIter_methods[] = {
	{NULL}
};

static PyTypeObject MapIterType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "map.MapIter",
	.tp_doc = map_obj_doc,
	.tp_basicsize = sizeof(MapIterObject),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,    /* Py_TPFLAGS_HAVE_ITER is set for iterator features */
	.tp_alloc = PyType_GenericAlloc,
	.tp_new = MapIter_new,
	.tp_init = (initproc)MapIter_init,
	.tp_dealloc = (destructor)MapIter_dealloc,                 /* destructor */
	.tp_members = MapIter_members,
	.tp_methods = MapIter_methods,
    // TODO: comment out after implemented
    // .tp_iter = ,
    // .tp_iternext = ,
};


/*
 * Configurations for map module.
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

	if (PyType_Ready(&MapIterType) < 0)
		return NULL;

    map = PyModule_Create(&mapmodule);
    if (map == NULL)
        return NULL;

    Py_INCREF(&MapType);
	Py_INCREF(&MapIterType);
    if (PyModule_AddObject(map, "Map", (PyObject *) &MapType) < 0) {
        Py_DECREF(&MapType);
        Py_DECREF(map);
        return NULL;
    }

	if (PyModule_AddObject(map, "MapIter", (PyObject *)&MapIterType) < 0) {
		Py_DECREF(&MapIterType);
		Py_DECREF(&MapType);
		Py_DECREF(map);
		return NULL;
	}

    return map;
}
