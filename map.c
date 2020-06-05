/* Game-of-live map implementation */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include "life_helpers.h"

#define MAP(obj) ((MapObject *)(obj))
#define ITER(obj) ((MapIterObject *)(obj))

PyObject *MapIter_IterNext(PyObject *self);
PyObject *MapIter_Iter(PyObject *self);
static PyObject *MapIter_New(PyTypeObject *type, PyObject *args, PyObject *kwds);
static PyTypeObject MapIterType;

/*
 * Helper function.
 */
static PyObject *list_to_PyTuple(int *map, int nrow, int ncol) {
	PyObject *res = PyTuple_New((Py_ssize_t) nrow);
	PyObject *row = NULL;
	for (int idx = 0; idx < nrow * ncol; idx++) {
		if (idx % ncol == 0) {
			row = PyTuple_New((Py_ssize_t) ncol);
			PyTuple_SetItem(res, idx / ncol, row);
		}
		PyTuple_SetItem(row, idx % ncol, PyLong_FromLong(map[idx]));
	}
	return res;
}

/*     
 * Documentation for map.
 */
PyDoc_STRVAR(Map_ModDoc, "This module implements the map of 2D version \
    of the 'Game of Life' and a generator of the map, which returns the \
    current state and updates the state of the map when called.\
	Import: from map import Map, MapIter");

PyDoc_STRVAR(Map_ObjDoc, "Usage: map.Map(row: int, col: int)");

/*
 * Custom PyObject for Map.
 */
typedef struct {
    PyObject_HEAD
    int *m;
    int nrow;
    int ncol;
} MapObject;

/* Map Iterator Class */
typedef struct {
	PyObject_HEAD
		PyObject *mobj;
	int *buf;
	int curr;
	int goal;
} MapIterObject;

static int MapIter_Init(MapIterObject *self, PyObject *args, PyObject *kwds);
static void MapIter_Dealloc(MapIterObject *self);

/* Since we have some data to manage, we at least need to have
   deallocation. */
static void Map_Dealloc(MapObject *self) {
    free(self->m);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject *Map_New(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    MapObject *self;
    /* allocate memory for self, and since we did not fill the tp_alloc slot,
       PyType_Ready() is used as default by object */
    self = (MapObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
	    self->m = NULL;
        self->nrow  = 0;
        self->ncol  = 0;
    } 
    return (PyObject *) self;
}

static int Map_Init(MapObject *self, PyObject *args, PyObject *kwds) {
    /* notice that in the previous function we have
       new-ed self hence we can use it here */
    /* parse args to three ints, third is optional
       doc: https://docs.python.org/3/c-api/arg.html */
    if (!PyArg_ParseTuple(args, "ll" /* only for ParseTuple*/,
	    &self->nrow, &self->ncol))
        return -1;
    return 0;
}

/*
 * Define all map's members.
 */
static PyMemberDef Map_Members[] = {
    {"nrow", T_INT, offsetof(MapObject, nrow), READONLY, "number of rows"},
    {"ncol", T_INT, offsetof(MapObject, ncol), READONLY, "number of columns"},
    {NULL}	/* Sentinal */
};

static PyObject *Map_SetMap(MapObject *self, PyObject *args, void *closure) {
    /* prevent from being garbage collected during this process. */
	Py_XINCREF(args);
    /* copy by value; DO NOT COPY BY REF use
     * unsigned index, whereas Py_ssize_t is signed
     */
    Py_ssize_t length = 0, idx = 0;
	PyObject *cell, *pMap;		/* no new ref */
    /* error checking */
	if (!PyArg_ParseTuple(args, "O!:", &PyList_Type, &pMap /* here pMap receives the actual obj passed, no new reference */)) {
		PyErr_SetString(PyExc_TypeError, "Parameter must be a list.");
		goto error;
    } else if (self->m != NULL) {
        PyErr_SetString(PyExc_TypeError, "Map has already been set. To set, initialize another instance.");
        goto error;
    } else if (self->ncol == 0 || self->nrow == 0) {
        PyErr_SetString(PyExc_AttributeError, "Attribute 'row' or 'col' is invalid(0).");
        goto error;
    } else if ((length = PyList_Size(pMap)) == 0) {
        PyErr_SetString(PyExc_ValueError, "Cannot use empty list to initialize map.");
        goto error;
    } else if (self->nrow * self->ncol != length) {
        PyErr_SetString(PyExc_IndexError, "Too many or too few elements.");
		goto error;
    }
    /* initialize map*/
    self->m = malloc(sizeof(int) * self->ncol * self->nrow);
    for (int idx = 0; idx < self->ncol * self->nrow; idx++) {
        if (!PyLong_Check((cell = PyList_GetItem(pMap, idx)) /* borrowed ref */)) {
            PyErr_SetString(PyExc_TypeError, "List elements must be integer valued.");
            goto error2;
        } else if (PyLong_AsLong(cell) != 0 && PyLong_AsLong(cell) != 1) {
            PyErr_SetString(PyExc_TypeError, "List elements must be either 0 or 1.");
            goto error2;
        } else {
            self->m[idx] = PyLong_AsLong(cell);
        }
    }
	Py_DECREF(args);
	Py_RETURN_NONE;

error:
	Py_XDECREF(args);
	return NULL;

error2:
	self->m = NULL;
	Py_XDECREF(args);
	return NULL;
}  

static PyObject *Map_GetMap(MapObject *self, void *closure) {
    if (self->m != NULL) {
        return list_to_PyTuple(self->m, self->nrow, self->ncol);
    } else {
        return Py_None;
    }
}

static PyObject *Map_Iter(MapObject *self, PyObject *args) {
    long int goal;
    if (!PyArg_ParseTuple(args, "l", &goal))
        return NULL;
    MapIterObject *iter = PyObject_New(MapIterObject, &MapIterType);
    if (!iter)
        return NULL;
    if (!PyObject_Init((PyObject *) iter, &MapIterType)) {
        Py_DECREF(iter);
        return NULL;
    }
    iter->goal = goal;
    iter->mobj = (PyObject *)self;
    Py_INCREF(self);
    return (PyObject *)iter;
}

/*
 * Define all Map's methods.
 */
static PyMethodDef Map_Methods[] = {
    {"set_map", (PyCFunction) Map_SetMap, METH_VARARGS,
     "Set the content of the map."},
    {"get_map", (PyCFunction) Map_GetMap, METH_NOARGS,
     "Get a tuple version of the map."},
	{"get_iter", (PyCFunction) Map_Iter, METH_VARARGS,
	 "Get a new iterator of the map object."},
    {NULL}
};

/*
 * Configurations for Map.
 */
static PyTypeObject MapType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "map.Map",
    .tp_doc  = Map_ObjDoc,
    .tp_basicsize = sizeof(MapObject),
    .tp_itemsize  = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,                         /* not subtypable */
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = Map_New,
    .tp_init = (initproc) Map_Init,
    .tp_dealloc = (destructor) Map_Dealloc,                 /* destructor */
    .tp_members = Map_Members,
    .tp_methods = Map_Methods,
};

static void MapIter_Dealloc(MapIterObject *self) {
	assert(!self->buf);
	free(self->buf);
    Py_XDECREF(self->mobj);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *MapIter_New(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	MapIterObject *self;
	self = (MapIterObject *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->curr = 0;
		self->goal = 0;
		self->buf = NULL;
		self->mobj = NULL;
	}
	return (PyObject *) self;
}

static int MapIter_Init(MapIterObject *self, PyObject *args, PyObject *kwds) {
	if (!PyArg_ParseTuple(args, "Ol" /* only for ParseTuple*/,
		&self->mobj, &self->goal))
		return -1;
    Py_ssize_t len = (Py_ssize_t) MAP(self->mobj)->ncol * MAP(self->mobj)->nrow;
    self->buf = malloc(sizeof(int) * len);
    for (int *p = MAP(self->mobj)->m, *q = self->buf; p - MAP(self->mobj)->m < len; *q++ = *p++);
	return 0;
}

static PyMemberDef MapIter_Members[] = {
	{"curr", T_INT, offsetof(MapIterObject, curr), READONLY, "current state"},
	{"goal", T_INT, offsetof(MapIterObject, goal), READONLY, "goal state"},
	{NULL}	/* Sentinal */
};

// Set tp_iternext to this function
// Python interpreter checks on whether the tp_iter is provided
// if no tp_iter is provided, it checks whether the type is sequence
// in this case, the MapIter is not of a sequence type. Therefore,
// fail to provide tp_iter cause "object is not iterable" exception.
// tp_iter should return another object that passes PyIter_Check.
// No new reference created.
PyObject *MapIter_Iter(PyObject *self) {
	if (!MAP(self)->m) {
		PyErr_SetString(PyExc_AssertionError, "Map object has not been initialized.");
		return NULL;
	}
    ITER(self)->curr = 0;
    Py_INCREF(self);
	return self;
}

// Set tp_iternext to this function
// tp_iternext should take an iterator, which would be the iterable
// returned by tp_iter. Return of this function creates new reference.
// Return NULL with no exception set when finished
PyObject *MapIter_IterNext(PyObject *self) {
	MapIterObject *p = ITER(self);
    MapObject *q = MAP(p);
    if (p->curr++ < p->goal) {
        update_map(p->buf, q->nrow, q->ncol);
        return list_to_PyTuple(p->buf, q->nrow, q->ncol);
    } else {
        /* return NULL is required */
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }
}

static PyTypeObject MapIterType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "map._MapIter",
	.tp_doc = Map_ModDoc,
	.tp_basicsize = sizeof(MapIterObject),
	.tp_itemsize = 0,
#if PY_MAJOR_VERSION >=3
	#define Py_TPFLAGS_HAVE_ITER 0
#endif
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,    /* Py_TPFLAGS_HAVE_ITER is set for iterator features */
	.tp_alloc = PyType_GenericAlloc,
	.tp_new = MapIter_New,
	.tp_init = (initproc)MapIter_Init,
	.tp_dealloc = (destructor)MapIter_Dealloc,                 /* destructor */
	.tp_members = MapIter_Members,
	.tp_iter = (getiterfunc)MapIter_Iter,
	.tp_iternext = (iternextfunc)MapIter_IterNext,
};

/*
 * Configurations for map module.
 */
static PyModuleDef MapModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "map",
    .m_doc = Map_ModDoc,
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

    map = PyModule_Create(&MapModule);
    if (map == NULL)
        return NULL;

    Py_INCREF(&MapType);
	Py_INCREF(&MapIterType);
    if (PyModule_AddObject(map, "Map", (PyObject *) &MapType) < 0) {
        Py_DECREF(&MapType);
        Py_DECREF(map);
        return NULL;
    }

	if (PyModule_AddObject(map, "_MapIter", (PyObject *)&MapIterType) < 0) {
		Py_DECREF(&MapIterType);
		Py_DECREF(&MapType);
		Py_DECREF(map);
		return NULL;
	}

    return map;
}
