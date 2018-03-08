#include <vapor/MyPython.h>

namespace VAPoR {
RENDER_API int Write_PNG(const char *file, int width, int height, unsigned char *buffer)
{
    PyObject *pName, *pModule, *pFunc, *pArgs, *pValue;

    Wasp::MyPython::Instance()->Initialize();

    pName = PyString_FromString("imagewriter");
    pModule = PyImport_Import(pName);

    if (pModule == NULL) {
        std::cerr << "pModule (drawpng) NULL!!" << std::endl;
        PyErr_Print();
        return 1;
    }
    pFunc = PyObject_GetAttrString(pModule, "drawpng");
    if (pFunc && PyCallable_Check(pFunc)) {
        pArgs = PyTuple_New(4);

        // The 1st argument: output filename
        pValue = PyString_FromString(file);
        PyTuple_SetItem(pArgs, 0, pValue);

        // The 2nd argument: width
        pValue = PyInt_FromLong((long)width);
        PyTuple_SetItem(pArgs, 1, pValue);

        // The 3rd argument: height
        pValue = PyInt_FromLong((long)height);
        PyTuple_SetItem(pArgs, 2, pValue);

        // The 4th argument: RGB buffer
        long      nChars = width * height * 3;
        PyObject *pListOfChars = PyList_New(nChars);
        assert(pListOfChars);
        for (long i = 0; i < nChars; i++) {
            int rt = PyList_SetItem(pListOfChars, i, PyInt_FromLong((long)buffer[i]));
            assert(rt == 0);
        }
        PyTuple_SetItem(pArgs, 3, pListOfChars);

        // Call the python routine
        pValue = PyObject_CallObject(pFunc, pArgs);
        if (pValue == NULL) {
            std::cerr << "pFunc failed to execute!!" << std::endl;
            PyErr_Print();
        }
    } else {
        std::cerr << "pFunc NULL" << std::endl;
        PyErr_Print();
        return 1;
    }

    Py_XDECREF(pName);
    Py_XDECREF(pArgs);
    Py_XDECREF(pValue);
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);

    return 0;
}
}    // namespace VAPoR
