#include <stdio.h>
#include <stdlib.h>
#include <Python.h>

void IncreaseArraySize(long ** array, size_t* size) {
	(*size)*=2;
	*array = realloc(*array,sizeof(long) *(*size));
}


PyObject* CreateListFromElements(long* array, size_t count) {
	PyObject* returned_value;
	if (count < 2) {
		returned_value = PyUnicode_FromString("Prime!");
	} else {
		returned_value = PyList_New(count+1);
		PyList_SetItem(returned_value, 0, PyLong_FromLong(1));

		for (size_t i = 0; i < count; ++i) {
			PyList_SetItem(returned_value, i+1, PyLong_FromLong(array[i]));
		}
	}
	free(array);
	return returned_value;
}

PyObject* FindPrimeDivisors(long n) {
	size_t arr_size = 4;
	long* primes = (long *)malloc(sizeof(long) * arr_size);
	size_t prime_count = 0;

	for (size_t i = 2; i*i<=n; ++i) {
		while (n%i == 0) {
			primes[prime_count] = i;
			++prime_count;
			if (prime_count > arr_size)
				IncreaseArraySize(&primes, &arr_size);
			n/=i;
		}
	}


	if (n>2) {
		primes[prime_count] = n;
		++prime_count;
		if (prime_count > arr_size)
			IncreaseArraySize(&primes, &arr_size);
	}


	return CreateListFromElements(primes, prime_count);

}

static PyObject* factor_out(PyObject *self, PyObject *args) {
	long input_number;
    if (! PyArg_ParseTuple(args, "l", &input_number) )
		 return NULL;

	return FindPrimeDivisors(input_number);
}

static PyMethodDef methods[] = {
	{
		.ml_name = "factor_out",
		.ml_meth = factor_out,
		.ml_flags = METH_VARARGS,
		.ml_doc = "Do something very useful"},
		{NULL, NULL, 0, NULL}
};

PyModuleDef moduleDef = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "primes",
    .m_size = -1,
    .m_methods = methods,
};

PyMODINIT_FUNC PyInit_primes()
{
	return PyModule_Create(&moduleDef);

}