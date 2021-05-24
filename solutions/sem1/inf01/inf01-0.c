#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <stdio.h>

#define TYPE int
PyObject* CompleteListWithZeros(PyObject* list, size_t len) {
	PyObject* new_list = PyList_New(len);
	
	size_t input_list_len = PyList_Size(list);

	for (size_t i = 0; i < input_list_len; ++i) {
		PyList_SetItem(new_list, i, PyList_GetItem(list, i));
	}


	for (size_t i = input_list_len; i < len; ++i) {
		PyList_SetItem(new_list, i, PyFloat_FromDouble(0));
	}

	return new_list;

}

PyObject* CreateListWithZeros(PyObject* list, size_t len) {
	size_t real_arr_len = PyList_Size(list);
//	printf("real arr len: %ld\n", real_arr_len); 

	
	for(size_t i = 0; i < len; ++i) {
		PyList_SetItem(list, i, PyFloat_FromDouble(0));
	}
/*
	for(size_t i = 0; i < len; ++i) {
		//PyList_SetItem(list, i, PyFloat_FromDouble(0));
		printf("%ld ", PyList_GetItem(list,i));
	}
*/
//	printf("\n");
	return list;
}


PyObject* CheckListCount(PyObject* list, size_t len) {
	size_t external_arr_len = PyList_Size(list);
	if (external_arr_len != len) {
		PyObject* new_list = PyList_New(len);
		// copy the contents to a new array of correct size
		for(size_t i = 0; i < external_arr_len; ++i) {
			PyList_SetItem(new_list, i, PyList_GetItem(list, i));
		}

//		printf("should add %ld array\n", len - external_arr_len);
		for (size_t i = 0; i<len-external_arr_len; ++i) {
			PyObject* zeros_list = PyList_New(len);
			zeros_list = CreateListWithZeros(zeros_list, len);
			PyList_SetItem(new_list, external_arr_len+i, zeros_list);
		}
		return new_list;	
	}	
	return list;
}

PyObject* CheckListLength(PyObject* list, size_t len) {
	PyObject* new_list = PyList_New(len); 
	for(size_t i = 0; i < len; ++i) {
		PyObject* selected_list = PyList_GetItem(list, i);
		size_t selected_list_size = PyList_Size(selected_list);
		
		if (selected_list_size != len) {
			PyObject* completed_list = CompleteListWithZeros(selected_list, len);		
			PyList_SetItem(new_list, i, completed_list);
		} else {
			PyList_SetItem(new_list, i, selected_list);
		}
	}
	return new_list;
}	



PyObject* CheckListCompleteness(PyObject* list, size_t len) {
	PyObject* final_list = CheckListCount(list, len);
//	printf("new_size: %ld\n", PyList_Size(final_list));
	// check if each inside list's length == len
	// if not, complete it with zeros
	return CheckListLength(final_list, len);
	
}

double* CreateNxNArray(PyObject* l, size_t len) {
//	printf("Creating an %ldx%ld array\n", len, len);

	double* arr = (double* )malloc(sizeof(double) * len * len);
//	printf("l1size: %ld\n", PyList_Size(l));
	l = CheckListCompleteness(l, len);
//	printf("l1size: %ld\n", PyList_Size(l));


	for (size_t i = 0; i < len; ++i){
//		printf("index: %ld\n", i);
		PyObject* list = PyList_GetItem(l, i);
		size_t ss = PyList_Size(list);
//		printf("size: %ld\n",ss);
		for (size_t j = 0; j < len; ++j) {
			PyObject* el = PyList_GetItem(list, j);
			arr[j+i*len] = PyFloat_AsDouble(el);
//			printf("%d at index %ld\n", arr[j+i*len], j+i*len);
		}
	}

	return arr;
}


void DisplaySquareMatrix(double* arr, size_t len) {
	for(size_t i = 0; i<len*len; ++i) {
		printf("%f ", arr[i]);
		printf(i%len==len-1 ?"\n" :""); 
	}

	printf("\n");
	
}

double* SquaredMatrixMultiplication(double* arr1, double* arr2, size_t len)
{
	// initialize the matrix with zeros;
	
	double* res = (double*)malloc(sizeof(double)*len*len);

	for(size_t i = 0; i<len*len; ++i) 
		res[i] = 0;
	
	for(size_t i = 0; i < len; ++i)
		for(size_t j = 0; j < len; ++j)
			for (size_t k = 0; k < len; ++k) 
				res[j+i*len] += arr1[k+i*len]*arr2[j+k*len];
//	printf("Multiplication result: \n");
//	DisplaySquareMatrix(res, len);
	return res;

}

PyObject* ConvertArrToPyList(double* res, size_t len) {

	PyObject* plist = PyList_New(len);

	for(size_t i = 0; i < len; ++i) {
		PyObject* plist1 = PyList_New(len);
		for (size_t j = 0; j < len; ++j) {
			PyList_SetItem(plist1, j, PyFloat_FromDouble(res[j+i*len]));
		}
		PyList_SetItem(plist, i, plist1);
	}

//	printf("here\n");
	return plist;
 
}

static PyObject * dot(PyObject *self, PyObject *args) {

	PyObject* list1;
	PyObject* list2;

	PyObject* initial_length;

//	printf("running module\n");

	
	if (! PyArg_ParseTuple(args, "OOO", &initial_length, &list1, &list2) )
		return NULL;


	size_t length = PyLong_AsSize_t(initial_length);
	double* arr1 = CreateNxNArray(list1, length);
	double* arr2 = CreateNxNArray(list2, length);

	double* res = SquaredMatrixMultiplication(arr1, arr2, length);
	PyObject* result_list = ConvertArrToPyList(res, length);

//	printf("here1\n");
	free(arr1);
	free(arr2);
	free(res);

	return result_list;
}

static PyMethodDef methods[] = {
	{
		.ml_name = "dot",
		.ml_meth = dot,
		.ml_flags = METH_VARARGS,
		.ml_doc = "Do something very useful"},
		{NULL, NULL, 0, NULL}
};

PyModuleDef moduleDef = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "matrix",
    .m_size = -1,
    .m_methods = methods,
};

PyMODINIT_FUNC PyInit_matrix()
{
return PyModule_Create(&moduleDef);

}
