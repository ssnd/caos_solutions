//
// Created by Sandu Kiritsa on 05/10/2020.
//

#include <stdio.h>
#include <Python.h>
#include <frameobject.h>

typedef struct CharArr
{
    char *data;
    size_t len;

} CharArr;

CharArr ReadStdinArray(char *fpath)
{
    CharArr result_char_arr;

    int arr_size = 64;
    int len = 0;
    char *arr = (char *)malloc(sizeof(char) * arr_size);

    FILE *fp = fopen(fpath, "r");
    if (fp == NULL)
    {
        exit(EXIT_FAILURE);
    }
    char c;
    while ((c = fgetc(fp)) != EOF)
    {
        if (len >= arr_size)
        {
            arr_size *= 2;
            arr = realloc(arr, sizeof(char) * arr_size);
        }
        arr[len] = c;
        len++;
    }
    arr[len] = '\0';
    fclose(fp);

    result_char_arr.len = len;
    result_char_arr.data = arr;

    return result_char_arr;
}

void Pop(PyObject *q)
{
    if (PyList_Size(q) > 0)
    {
        //		printf("pop\n");
        PyList_SetSlice(q, 0, 1, NULL);
    }
}

long next_action(PyObject *q)
{
    PyObject *info_tuple = PyList_GetItem(q, 0);

    return PyLong_AsLong(PyTuple_GetItem(info_tuple, 0));
}

Py_UCS4 *next_script(PyObject *q)
{
    PyObject *info_tuple = PyList_GetItem(q, 0);
    return PyUnicode_AsUCS4Copy(PyTuple_GetItem(info_tuple, 1));
}
static int a = 0;
static int called = 0;
static int in_func = 0;

int trace(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
    size_t lineno = PyFrame_GetLineNumber(frame);
    Py_UNICODE *fname = PyUnicode_AsUnicode(frame->f_code->co_filename);
    if (fname[0] != '<')
    {
        return 0;
    }

    PyObject *buff = PyList_GetItem(obj, 1);
    PyObject *state = PyList_GetItem(obj, 2);
    PyObject *q = PyList_GetItem(obj, 0);

    //PyObject* info_tuple = PyList_GetItem(q, 0);
    //long next_action = PyLong_AsLong(PyTuple_GetItem(info_tuple, 0));
    if (PyList_Size(q) == 0)
        return 0;
    //
    //	printf("what: %d, line:%ld, q size: %ld\n" ,what,PyFrame_GetLineNumber(frame),PyList_Size(q));

    if (what == PyTrace_CALL)
    {
        if (a == 0)
        {
            a = 1;
            return 0;
        }
        if (next_action(q) == 3L)
        {
            called++;
            //	printf("step in function\n");
            in_func = 1;
            Pop(q);
            return 0;
        }
        in_func = 1;
    }

    if (what == PyTrace_LINE)
    {
        if (in_func == 0 && called == 0 && next_action(q) == 2L)
        {
            //	printf("next line\n");
            Pop(q);
            return 0;
        }
        if (in_func == 1 && called == 0)
        {
            //	printf("waiting for the function to return a value\n");
        }

        // in function and need to read every line
        if (in_func == 1 && called == 1 && next_action(q) == 2L)
        {
            //	printf("stepped in: tracing every line of the function\n");
            Pop(q);
        }

        if (next_action(q) == 4L)
        {
            //	printf("running some script here\n");
            Py_UCS4 *s = next_script(q);
            char *command = (char *)malloc(sizeof(char) * 64);

            sprintf(command, "print(repr(%ls))", s);

            //	printf("printed : %s\n", command);
            PyRun_String(command, Py_single_input, PyEval_GetGlobals(), PyEval_GetLocals());
            free(command);
            Pop(q);
            return 0;
        }
    }

    if (what == PyTrace_RETURN)
    {
        if (in_func == 1 && called == 0)
        {
            in_func = 0;
        }

        if (in_func == 1 && called > 0 && next_action(q) == 2L)
        {
            //			printf("popping return\n");
            in_func = 0;
            called--;

            Pop(q);

            if (in_func == 0 && called == 0 && next_action(q) == 4L)
            {
                //			printf("running some script here\n");

                Py_UCS4 *s = next_script(q);
                char *command = (char *)malloc(sizeof(char) * 64);

                sprintf(command, "print(repr(%ls))", s);

                //			printf("printed : %s\n", command);
                PyRun_String(command, Py_single_input, PyEval_GetGlobals(), PyEval_GetLocals());
                free(command);

                Pop(q);
            }

            if (in_func == 0 && called == 0 && next_action(q) == 2L)
            {
                //			printf("next after return\n");
                Pop(q);
            }
        }
        return 0;
    }

    return 0;
}

PyObject *CreateTuple(size_t val, PyObject *command)
{
    PyObject *tuple = PyTuple_New(2);

    PyTuple_SetItem(tuple, 0, PyLong_FromLong(val));

    PyTuple_SetItem(tuple, 1, command);

    return tuple;
}

int main(int argc, char *argv[])
{
    Py_Initialize();
    PyObject *list = PyList_New(0);
    PyObject *dbg_obj = PyList_New(3);
    if (argc < 2)
    {
        exit(EXIT_FAILURE);
    }
    char *pyfile;
    char *dbgfile;

    wchar_t **_argv = PyMem_Malloc(sizeof(wchar_t *) * (argc - 3));
    for (int i = 0; i < argc; i++)
    {
        wchar_t *arg = Py_DecodeLocale(argv[i], NULL);
        if (i == 1)
        {
            //            _argv[0] = arg;
            dbgfile = argv[1];
        }
        if (i == 2)
        {
            pyfile = argv[2];
        }
        if (i > 2)
        {
            _argv[i - 3] = arg;
        }
    }

    PySys_SetArgv(argc - 3, _argv);

    CharArr code = ReadStdinArray(pyfile);
    CharArr dbg_text = ReadStdinArray(dbgfile);
    PyObject *dbg_string = PyUnicode_FromStringAndSize(dbg_text.data, dbg_text.len);

    PyObject *lines = PyUnicode_Splitlines(dbg_string, 0);

    for (size_t line_index = 0; line_index < (size_t)PyList_Size(lines); ++line_index)
    {

        PyObject *uni_cmd = PyList_GetItem(lines, line_index);

        if (PyUnicode_Contains(uni_cmd, PyUnicode_FromString("next")))
        {
            PyList_Append(list, CreateTuple(2L, NULL));
            continue;
        }
        if (PyUnicode_Contains(uni_cmd, PyUnicode_FromString("step")))
        {
            PyList_Append(list, CreateTuple(3L, NULL));
            continue;
        }
        if (PyUnicode_Contains(uni_cmd, PyUnicode_FromString("continue")))
        {
            break;
        }
        if (PyUnicode_Contains(uni_cmd, PyUnicode_FromString("print")))
        {
            PyObject *split_list = PyUnicode_Split(uni_cmd, PyUnicode_FromString("print"), -1);

            PyObject *run_str = PyList_GetItem(split_list, 1);

            PyList_Append(list, CreateTuple(4L, run_str));
        }
    }

    PyObject *state_dict = PyDict_New();

    PyDict_SetItemString(state_dict, "in_func", PyLong_FromLong(0));
    PyDict_SetItemString(state_dict, "called", PyLong_FromLong(0));
    PyList_SetItem(dbg_obj, 0, list);
    PyList_SetItem(dbg_obj, 1, PyList_New(0));
    PyList_SetItem(dbg_obj, 2, state_dict);

    PyEval_SetTrace(trace, dbg_obj);
    PyRun_SimpleString(code.data);
    free(code.data);
    free(dbg_text.data);

    PyMem_Free(_argv);
    Py_Finalize();

    return 0;
}