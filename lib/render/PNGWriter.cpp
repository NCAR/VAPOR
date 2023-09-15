#include "vapor/PNGWriter.h"
#include "vapor/VAssert.h"

#define USE_PYTHON_PNG 1

#if USE_PYTHON_PNG
    #include "vapor/MyPython.h"
#else
    #include <png.h>
#endif

using namespace VAPoR;
using namespace Wasp;

REGISTER_IMAGEWRITER(PNGWriter);

std::vector<std::string> PNGWriter::GetFileExtensions() { return {"png"}; }

PNGWriter::PNGWriter(const string &path) : ImageWriter(path) {}

int PNGWriter::Write(const unsigned char *buffer, const unsigned int width, const unsigned int height)
{
#if USE_PYTHON_PNG

    VAssert(format == Format::RGB);

    PyObject *pName, *pModule, *pFunc, *pArgs, *pValue;

    int rc = Wasp::MyPython::Instance()->Initialize();
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to initialize python : %s", MyPython::Instance()->PyErr().c_str());
        return (-1);
    }

    pName = PyUnicode_FromString("imagewriter");
    pModule = PyImport_Import(pName);

    if (pModule == NULL) {
        PyErr_Print();
        MyBase::SetErrMsg("pModule (drawpng) NULL : %s", MyPython::Instance()->PyErr().c_str());
        return -1;
    }
    pFunc = PyObject_GetAttrString(pModule, "drawpng");
    if (pFunc && PyCallable_Check(pFunc)) {
        pArgs = PyTuple_New(4);

        // The 1st argument: output filename
        pValue = PyUnicode_FromString(path.c_str());
        PyTuple_SetItem(pArgs, 0, pValue);

        // The 2nd argument: width
        pValue = PyLong_FromLong((long)width);
        PyTuple_SetItem(pArgs, 1, pValue);

        // The 3rd argument: height
        pValue = PyLong_FromLong((long)height);
        PyTuple_SetItem(pArgs, 2, pValue);

        // The 4th argument: RGB buffer
        long      nChars = width * height * 3;
        PyObject *pListOfChars = PyList_New(nChars);
        VAssert(pListOfChars);
        for (long i = 0; i < nChars; i++) {
            int rt = PyList_SetItem(pListOfChars, i, PyLong_FromLong((long)buffer[i]));
            VAssert(rt == 0);
        }
        PyTuple_SetItem(pArgs, 3, pListOfChars);

        // Call the python routine
        pValue = PyObject_CallObject(pFunc, pArgs);
        if (pValue == NULL) {
            PyErr_Print();
            MyBase::SetErrMsg("pFunc (drawpng) failed to execute : %s", MyPython::Instance()->PyErr().c_str());
            return -1;
        }
    } else {
        PyErr_Print();
        MyBase::SetErrMsg("pFunc (drawpng) NULL : %s", MyPython::Instance()->PyErr().c_str());
        return -1;
    }

    Py_XDECREF(pName);
    Py_XDECREF(pArgs);
    Py_XDECREF(pValue);
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);

    return 0;
#else
    int         code = 0;
    FILE *      fp = NULL;
    png_structp png_ptr = NULL;
    png_infop   info_ptr = NULL;
    png_bytep   row = NULL;

    fp = fopen(file, "wb");
    if (!fp) return -1;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) VAssert(0);

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) VAssert(0);

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    #error Functionality Not Finished

    png_write_end(png_ptr, NULL);
    if (fp) fclose(fp);
    if (info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

    return 0;
#endif
}
