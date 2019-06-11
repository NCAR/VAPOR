#include <iostream>
#include <string>
#include <vector>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/EasyThreads.h>
#include <vapor/FileUtils.h>

using namespace Wasp;

struct {
    int n;
    int nthreads;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"n", 1, "1024", "Problem size"},
                                         {"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"n", Wasp::CvtToInt, &opt.n, sizeof(opt.n)}, {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)}, {NULL}};

const char *ProgName;

float *Matrix1, *Matrix2, *Result;
size_t N = 0;

// This function multiplies
// mat1 and mat2, and
// stores the result in res
void multiply(float *mat1, float *mat2, float *res, size_t n, size_t offset, size_t length)
{
    int i, j, k;
    for (i = offset; i < offset + length; i++) {
        for (j = 0; j < n; j++) {
            res[i * n + j] = 0;
            for (k = 0; k < n; k++) res[i * n + j] += mat1[i * n + k] * mat2[k * n + j];
        }
    }
}

void init_matrix(int n)
{
    N = n;

    Matrix1 = new float[n * n];
    Matrix2 = new float[n * n];
    Result = new float[n * n];

    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            Matrix1[j * n + i] = i;
            Matrix2[j * n + i] = j;
            Result[j * n + i] = 0.0;
        }
    }
}

float matrix_sum(int n)
{
    double sum = 0.0;

    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) { sum += Result[j * n + i]; }
    }
    return ((float)sum);
}

typedef struct {
    int _id;
    int _offset;
    int _length;
} thread_args_t;

void *my_thread_func(void *arg)
{
    thread_args_t &thread_arg = *(thread_args_t *)arg;

    cout << "Thread id, offset, length " << thread_arg._id << " " << thread_arg._offset << " " << thread_arg._length << endl;

    multiply(Matrix1, Matrix2, Result, N, thread_arg._offset, thread_arg._length);

    return (NULL);
}

int main(int argc, char **argv)
{
    OptionParser op;
    string       s;

    ProgName = FileUtils::LegacyBasename(argv[0]);

    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (argc != 1) {
        cerr << "Usage: " << ProgName << " [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    init_matrix(opt.n);

    int nthreads = opt.nthreads;
    if (nthreads < 1) nthreads = EasyThreads::NProc();
    if (nthreads < 1) nthreads = 1;

    cout << "Requesting " << nthreads << " threads" << endl;

    EasyThreads et(nthreads);

    cout << "Got " << et.GetNumThreads() << " threads" << endl;

    thread_args_t *     thread_args_ptr = new thread_args_t[et.GetNumThreads()];
    std::vector<void *> argvec;
    for (int i = 0; i < et.GetNumThreads(); i++) {
        thread_args_ptr[i]._id = i;
        et.Decompose(opt.n, et.GetNumThreads(), i, &(thread_args_ptr[i]._offset), &(thread_args_ptr[i]._length));
        argvec.push_back((void *)&thread_args_ptr[i]);
    }

    et.ParRun(my_thread_func, argvec);

    cout << "Matrix sum " << matrix_sum(opt.n) << endl;

    return (0);
}
