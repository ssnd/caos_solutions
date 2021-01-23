#include <stdio.h>
#include <stdlib.h>
// В этом заголовочном файле собран почти весь API Python
#include <Python.h>

typedef struct CharArr
{
    char *data;
    size_t len;
} CharArr;

typedef struct Pos
{
    char first;
    int second;
} Pos;

typedef struct Pair
{
    int first;
    int second;
} Pair;

CharArr ReadStdinArray()
{
    CharArr result_char_arr;

    int arr_size = 64;
    int len = 0;
    char *arr = (char *)malloc(sizeof(char) * arr_size);

    char c;
    while ((c = getchar()) != EOF)
    {
        if (len >= arr_size)
        {
            arr_size *= 2;
            arr = realloc(arr, sizeof(char) * arr_size);
        }
        arr[len] = c;
        len++;
    }

    result_char_arr.len = len;
    result_char_arr.data = arr;

    return result_char_arr;
}

void DisplayCell(Pos c)
{
    printf("%c%d\n", c.first, c.second);
}

Pos GetCellName(int i, int j)
{
    Pos a;
    a.first = i + 'A';
    a.second = j + 1;
    return a;
}

PyObject *GetDelimChar()
{
    char delimiter[1] = {';'};
    return PyUnicode_FromStringAndSize(&(delimiter[0]), 1);
}

void AssignValue(Pos position, PyObject *value, PyObject *locals)
{
    char *run_string = (char *)malloc(sizeof(char) * 2);

    sprintf(run_string, "%c%d", position.first, position.second);
    PyDict_SetItemString(locals, run_string, value);

    free(run_string);
}

void ProcessCellValues(Pos position, PyObject *cell_contents, PyObject *locals)
{
    if (PyUnicode_AsWideChar(cell_contents, NULL, 0) - 1 == 0)
    {
        PyObject *empty_cell = PyUnicode_New(0, 0);
        AssignValue(position, empty_cell, locals);
        //		printf("empty cell...\n");
        return;
    }

    if (PyUnicode_ReadChar(cell_contents, 0) == '=')
    {
        //		AssignValue(position, cell_contents, locals);
        //		printf("formula found, skipping for now...\n");
        return;
    }

    if (PyUnicode_ReadChar(cell_contents, 0) == '"')
    {
        //		printf("string found...\n");

        //		printf("cell %c%d value = %ls\n", position.first,
        //										position.second,
        //										PyUnicode_AsUnicode(cell_contents));
        AssignValue(position, cell_contents, locals);
        return;
    }

    // otherwise it's an integer
    //Py_UNICODE *unicode_cell = PyUnicode_AsUnicodeCopy(cell_contents);
    //	size_t unicode_cell_len = PyUnicode_AsWideChar(cell_contents,NULL,0)-1;
    PyObject *int_value = PyLong_FromUnicodeObject(cell_contents, 10);
    AssignValue(position, int_value, locals);

    //	printf("cell %c%d value = %ld\n", position.first,
    //									  position.second,
    //									  PyLong_AsLong(int_value));
}

void ProcessFormulas(Pos position, PyObject *cell_contents, PyObject *locals, PyObject *globals)
{
    size_t unicode_cell_len = PyUnicode_AsWideChar(cell_contents, NULL, 0) - 1;
    if (unicode_cell_len && PyUnicode_ReadChar(cell_contents, 0) != '=')
    {
        return;
    }

    Py_UCS4 *unicode_cell = PyUnicode_AsUCS4Copy(cell_contents);

    char *variable_name = (char *)malloc(sizeof(char) * 2);
    char *run_string = (char *)malloc(sizeof(char) * 2 + sizeof(Py_UCS4) * unicode_cell_len);

    sprintf(variable_name, "%c%d", position.first, position.second);
    sprintf(run_string, "%s%ls", variable_name, (wchar_t *)unicode_cell);

    PyRun_String(run_string, Py_file_input, globals, locals);
    PyErr_Clear();

    free(variable_name);
    free(run_string);
    /*
PyObject*    py_main = PyImport_AddModule("__main__");
	  PyObject*  py_dict = PyModule_GetDict(py_main);
	  PyRun_String("print('1');D2=B2*C1;print('2');print('d2=',D2)", Py_single_input, py_dict, locals);
	  

	*/
}

void ProcessLine(PyObject *lines, PyObject *locals, PyObject *globals, int process_formulas)
{
    size_t line_count = PyList_Size(lines);

    for (size_t line_index = 0; line_index < line_count; ++line_index)
    {
        PyObject *line = PyList_GetItem(lines, line_index);
        PyObject *cells = PyUnicode_Split(line, GetDelimChar(), -1);

        size_t cell_count = PyList_Size(cells);

        for (size_t cell_index = 0; cell_index < cell_count; ++cell_index)
        {
            Pos position = GetCellName(cell_index, line_index);
            PyObject *cell_contents = PyList_GetItem(cells, cell_index);
            if (!process_formulas)
            {

                ProcessCellValues(position, cell_contents, locals);
                continue;
            }
            ProcessFormulas(position, cell_contents, locals, globals);
        }
    }
}
void ProcessInputArr(CharArr arr)
{
    PyObject *locals = PyDict_New();

    PyObject *py_main = PyImport_AddModule("__main__");
    PyObject *globals = PyModule_GetDict(py_main);

    PyObject *input_string = PyUnicode_FromStringAndSize(arr.data, arr.len);

    PyObject *lines = PyUnicode_Splitlines(input_string, 0);

    size_t line_count = PyList_Size(lines);

    PyObject *line = PyList_GetItem(lines, 0);
    PyObject *cells = PyUnicode_Split(line, GetDelimChar(), -1);

    size_t cell_count = PyList_Size(cells);

    int process_formulas = 0;
    ProcessLine(lines, locals, globals, process_formulas);
    process_formulas = 1;
    for (size_t i = 0; i < 2; ++i)
    {
        ProcessLine(lines, locals, globals, process_formulas);
    }

    ProcessLine(lines, locals, globals, process_formulas);
    for (size_t line_index = 0; line_index < line_count; ++line_index)
    {
        for (size_t cell_index = 0; cell_index < cell_count; ++cell_index)
        {
            Pos position = GetCellName(cell_index, line_index);
            char *run_string = (char *)malloc(sizeof(char) * 32);
            char *variable_name = (char *)malloc(sizeof(char) * 2);

            sprintf(variable_name, "%c%d", position.first, position.second);

            sprintf(run_string, "print(%s,end='')", variable_name);

            PyRun_String(run_string, Py_single_input, globals, locals);
            if (cell_index < cell_count - 1)
                PyRun_String("print(';', end='')", Py_single_input, globals, locals);
            PyErr_Clear();
            free(run_string);
            free(variable_name);
        }
        PyRun_String("print( '' )", Py_single_input, globals, locals);
    }
}

int main()
{
    CharArr input_arr = ReadStdinArray();

    Py_Initialize();

    ProcessInputArr(input_arr);

    Py_Finalize();

    free(input_arr.data);
}