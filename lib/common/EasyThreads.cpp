#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iostream>
#ifndef WIN32
#include <unistd.h>
#endif
#include <vapor/EasyThreads.h>
//#include <vapor/MyBase.h>

#include <cassert>
using namespace Wasp;

#ifdef ENABLE_THREADS
#ifdef WIN32

typedef void *(*tfuncp)(void *);
DWORD WINAPI runner(void *arg) {
    void **info = (void **)arg;
    tfuncp func = (tfuncp)info[0];
    //if(info[1]) info[1] = *((void**)info[1]);
    HANDLE *mutices = (HANDLE *)info[2];
    int nthreads = (int)info[3];
    //lock one of the notifier mutices
    for (int i = 0; i < nthreads; i++) {
        if (WaitForSingleObject(mutices[i], 0) == 0)
            break;
        if (i == nthreads - 1)
            printf("EasyThreads: Failed to lock block mutex!");
    }
    //run the function
    func(info[1]);
    delete[] arg;
    for (int i = 0; i < nthreads; i++) {
        if (ReleaseMutex(mutices[i]) == TRUE)
            break;
    }
    return 0;
}

#endif
#endif

EasyThreads::EasyThreads(
    int nthreads) {

#ifndef WIN32
    nthreads_c = 0;
    threads_c = NULL;
    block_c = 0;
    count_c = 0;
#else
    nthreads_c = 0;
    threads_c = NULL;
    initialized_c = false;
    nblocked_c = 0;
    mutices_c = NULL;
    bMutices_c = NULL;
    mutex_c = NULL;
    bMutex_c = NULL;
#endif

#ifdef ENABLE_THREADS
    if (nthreads < 1)
        nthreads = NProc();
    if (char *s = getenv("VAPOR_NTHREADS")) {
        istringstream ist(s);
        ist >> nthreads;
        cout << "VAPOR_NTHREADS = " << nthreads << endl;
    }
#ifndef WIN32
    int rc;
    threads_c = NULL;
    block_c = 0;
    count_c = 0;
    nthreads_c = nthreads;

    rc = pthread_attr_init(&attr_c);
    if (rc < 0) {
        SetErrMsg("pthread_attr_init() : %s", strerror(errno));
        return;
    }

    rc = pthread_cond_init(&cond_c, NULL);
    if (rc < 0) {
        SetErrMsg("pthread_cond_init() : %s", strerror(errno));
        return;
    }

    rc = pthread_mutex_init(&barrier_lock_c, NULL);
    if (rc < 0) {
        SetErrMsg("pthread_mutex_init() : %s", strerror(errno));
        return;
    }

    rc = pthread_mutex_init(&mutex_lock_c, NULL);
    if (rc < 0) {
        SetErrMsg("pthread_mutex_init() : %s", strerror(errno));
        return;
    }

    pthread_attr_setdetachstate(&attr_c, PTHREAD_CREATE_JOINABLE);
    if (rc < 0) {
        SetErrMsg("pthread_attr_setdetachstate() : %s", strerror(errno));
        return;
    }

#ifdef __sgi
    rc = pthread_attr_setscope(&attr_c, PTHREAD_SCOPE_BOUND_NP);
#else
    rc = pthread_attr_setscope(&attr_c, PTHREAD_SCOPE_SYSTEM);
#endif
    if (rc < 0) {
        SetErrMsg("pthread_attr_setscope() : %s", strerror(errno));
        return;
    }

    threads_c = new pthread_t[nthreads_c];

#else //WIN32

    //make sure we know if initialization failed.
    initialized_c = false;
    //initialize basic fields
    nthreads_c = nthreads;
    nblocked_c = 0;
    //initialize threads and mutices
    threads_c = new HANDLE[nthreads_c];
    mutices_c = new HANDLE[nthreads_c];
    bMutices_c = new HANDLE[nthreads_c];
    for (int i = 0; i < nthreads_c; i++) {
        //Set up each mutex. If it's NULL, exit without setting initialized
        if ((mutices_c[i] = CreateMutex(NULL, FALSE, NULL)) == NULL) {
            SetErrMsg("EasyThreads: Failed to initialize mutices (notifier)!\n");
            return;
        }
    }
    for (int i = 0; i < nthreads_c; i++) {
        //Set up each mutex. If it's NULL, exit without setting initialized
        if ((bMutices_c[i] = CreateMutex(NULL, FALSE, NULL)) == NULL) {
            SetErrMsg("EasyThreads: Failed to initialize mutices (blocker)!\n");
            return;
        }
    }
    if ((mutex_c = CreateMutex(NULL, FALSE, "main_mutex")) == NULL) {
        SetErrMsg("EasyThreads: Failed to initialize mutices (main)!\n");
        return;
    }
    if ((bMutex_c = CreateMutex(NULL, FALSE, "barrier_mutex")) == NULL) {
        SetErrMsg("EasyThreads: Failed to initialize mutices (barrier)!\n");
        return;
    }
    //initialization succeeded!
    initialized_c = true;

#endif //OS-switch

#endif //ENABLE_THREADS
}

EasyThreads::~EasyThreads() {
#ifdef ENABLE_THREADS

#ifndef WIN32 //Mac, Linux

    pthread_attr_destroy(&attr_c);
    if (threads_c)
        delete[] threads_c;
    threads_c = NULL;

#else //Windows

    //close any mutices
    for (int i = 0; i < nthreads_c && mutices_c[i] != NULL; i++) {
        CloseHandle(mutices_c[i]);
    }
    for (int i = 0; i < nthreads_c && bMutices_c[i] != NULL; i++) {
        CloseHandle(bMutices_c[i]);
    }
    if (mutex_c != NULL)
        CloseHandle(mutex_c);
    if (bMutex_c != NULL)
        CloseHandle(bMutex_c);
    //deallocate arrays
    delete[] threads_c;
    delete[] bMutices_c;
    delete[] mutices_c;

#endif //OS-switch

#endif //ENABLE_THREADS
}

int EasyThreads::ParRun(
    void *(*start)(void *),
    void **arg) {
    vector<void *> argvec;
    for (int i = 0; i < nthreads_c; i++)
        argvec.push_back(arg[i]);

    return (EasyThreads::ParRun(start, argvec));
}

int EasyThreads::ParRun(
    void *(*start)(void *),
    std::vector<void *> argvec) {
#ifdef ENABLE_THREADS

#ifndef WIN32
    int i;
    int rc;
    int status = 0;

    for (i = 0; i < nthreads_c; i++) {
        rc = pthread_create(&threads_c[i], &attr_c, start, argvec[i]);
        if (rc < 0) {
            SetErrMsg("pthread_create() : %s", strerror(errno));
            return (-1);
        }
    }

    for (i = 0; i < nthreads_c; i++) {
        rc = pthread_join(threads_c[i], NULL);
        if (rc < 0) {
            SetErrMsg("pthread_join() : %s", strerror(errno));
            status = rc;
        }
    }
    return (status);

#else //WIN32

    if (!initialized_c)
        return -1;
    for (int i = 0; i < nthreads_c; i++) {
        //parse the arguments into a package for the runner
        void **info = new void *[4]; //TODO: Move to stack
        info[0] = (void *)start;
        info[1] = argvec[i];
        info[2] = (void *)mutices_c;
        info[3] = (void *)nthreads_c;
        //launch the runners
        if ((threads_c[i] = CreateThread(NULL, 0, runner, (void *)info, 0, NULL)) == NULL) {
            //if any of them fail, close all of them and return an error code.
            for (; i >= 0; i--) {
                if (TerminateThread(threads_c[i], 0) == FALSE) {
                    printf("EasyThreads: Failed to terminate thread after some failed to start!\n");
                }
            }
            delete[] info;
            SetErrMsg("EasyThreads: Failed to start threads.\n");
            return -1;
        }
    }

    //wait for the threads to finish, and then return success
    WaitForMultipleObjects(nthreads_c, threads_c, TRUE, INFINITE);

    return 0;

#endif

#else
    return 0;
#endif
}

int EasyThreads::Barrier() {
#ifdef ENABLE_THREADS

#ifndef WIN32

    int local;
    int rc;

    if (nthreads_c > 1) {
        rc = pthread_mutex_lock(&barrier_lock_c);
        if (rc < 0) {
            SetErrMsg("pthread_mutex_lock() : %s", strerror(errno));
            return (-1);
        }
        local = count_c;
        block_c++;
        if (block_c == nthreads_c) {
            block_c = 0;
            count_c++;
            rc = pthread_cond_broadcast(&cond_c);
            if (rc < 0) {
                SetErrMsg("pthread_cond_broadcast() : %s", strerror(errno));
                return (-1);
            }
        }
        while (local == count_c) {
            rc = pthread_cond_wait(&cond_c, &barrier_lock_c);
            if (rc < 0) {
                SetErrMsg("pthread_cond_wait() : %s", strerror(errno));
                return (-1);
            }
        }
        rc = pthread_mutex_unlock(&barrier_lock_c);
        if (rc < 0) {
            SetErrMsg("pthread_mutex_unlock() : %s", strerror(errno));
            return (-1);
        }
    }

#else //WIN32
    //In the windows implementation, the first thread to arrive locks all of the barrier mutices, then lets the other threads in.
    //The other threads then release their notifier mutices to notify the first thread that they've arrived.
    //Once the first thread owns all the notifier mutices, it releases the blocker and notifier mutices, and the other threads now each own a blocker mutex.
    //When the other threads own a blocker mutex, they release it and relock their notifier mutices.
    int status = 0;
    if (!initialized_c)
        return -1;
    if (nthreads_c == 1)
        return 0;
    if (WaitForSingleObject(bMutex_c, INFINITE) != 0) {
        printf("EasyThreads: Failed to lock barrier mutex!\n");
    }
    nblocked_c++;
    //printf("NUM: %d\n", nblocked_c);
    //if we arrived first, set up the wait sequence for the others.
    if (nblocked_c == 1) {
        //printf("First thread has arrived at barrier.\n");
        if (WaitForMultipleObjects(nthreads_c, bMutices_c, TRUE, 0) != 0) {
            printf("EasyThreads: Failed to lock blocker mutices in barrier (early)!\n");
            status |= 1;
        }
        //printf("First thread has locked the blocker mutices.\n");
        if (ReleaseMutex(bMutex_c) == 0) {
            printf("EasyThreads: Failed to release barrier function mutex (early)!\n");
            status |= 2;
        }
        //printf("First thread has released the barrier function mutex.\n");
        if (WaitForMultipleObjects(nthreads_c, mutices_c, TRUE, INFINITE) != 0) {
            printf("EasyThreads: Failed to lock notifier mutices in barrier (early)!\n");
            status |= 4;
        }
        //printf("First thread has locked the notifier mutices.\n");
        for (int i = 0; i < nthreads_c; i++) {
            if (ReleaseMutex(mutices_c[i]) == 0) {
                printf("EasyThreads: Failed to release notifier mutices in barrier (early)!\n");
                status |= 8;
            }
        }
        //printf("First thread has unlocked the notifier mutices.\n");
        for (int i = 0; i < nthreads_c; i++) {
            if (ReleaseMutex(bMutices_c[i]) == 0) {
                printf("EasyThreads: Failed to release blocker mutices in barrier (early)!\n");
                status |= 16;
            }
        }
        //printf("First thread has unlocked the blocker mutices.\n");
    } else {
        int n;
        for (n = 0; n < nthreads_c; n++) {
            if (ReleaseMutex(mutices_c[n]) != 0)
                break;
            if (n == nthreads_c - 1) {
                printf("EasyThreads: Failed to release notifier mutex (late)!\n");
                status |= 32;
            }
        }
        //printf("Thread %d has released its notifier mutex.\n", n);
        if (ReleaseMutex(bMutex_c) == 0) {
            printf("EasyThreads: Failed to release barrier function mutex (late)!\n");
            status |= 64;
        }
        //printf("Thread %d has released the barrier function mutex.\n", n);
        if (WaitForSingleObject(bMutices_c[n], INFINITE) != 0) {
            printf("EasyThreads: Failed to lock blocker mutex in barrier (late)!\n");
            status |= 128;
        }
        //printf("Thread %d has locked its blocker mutex.\n", n);
        if (ReleaseMutex(bMutices_c[n]) == 0) {
            printf("EasyThreads: Failed to release blocker mutex in barrier (late)!\n");
            status |= 256;
        }
        //printf("Thread %d has released its blocker mutex.\n", n);
        if (WaitForSingleObject(mutices_c[n], 0) != 0) {
            printf("EasyThreads: Unable to re-lock notifier mutex (late)!\n");
            status |= 512;
        }
        //printf("Thread %d has locked its notifier mutex.\n", n);
    }
    nblocked_c--;
    return -status;

#endif

#endif //ENABLE_THREADS
    return (0);
}

int EasyThreads::MutexLock() {
#ifdef ENABLE_THREADS

#ifndef WIN32

    if (nthreads_c > 1) {
        int rc = pthread_mutex_lock(&mutex_lock_c);
        if (rc < 0) {
            SetErrMsg("pthread_mutex_lock() : %s", strerror(errno));
            return (-1);
        }
    }

#else //WIN32

    if (!initialized_c)
        return -1;
    int result = WaitForSingleObject(mutex_c, INFINITE);
    return result != 0 ? -1 : 0;

#endif

#endif
    return (0);
}

int EasyThreads::MutexUnlock() {
#ifdef ENABLE_THREADS

#ifndef WIN32

    if (nthreads_c > 1) {
        int rc = pthread_mutex_unlock(&mutex_lock_c);
        if (rc < 0) {
            SetErrMsg("pthread_mutex_unlock() : %s", strerror(errno));
            return (-1);
        }
    }

#else

    if (!initialized_c)
        return -1;
    return (int)ReleaseMutex(mutex_c) - 1;

#endif

#endif
    return (0);
}

void EasyThreads::Decompose(
    int n,
    int size,
    int rank,
    int *offset,
    int *length) {
    int remainder = n % size;
    int chunk = n / size;

    if (rank < remainder) {
        *length = chunk + 1;
        *offset = (chunk + 1) * rank;
    } else {
        *length = chunk;
        *offset = (chunk + 1) * remainder + (rank - remainder) * chunk;
    }
}
int EasyThreads::NProc() {
#ifdef ENABLE_THREADS

#ifndef WIN32

#ifdef __sgi
    return (sysconf(_SC_NPROC_ONLN));
#else
    return (sysconf(_SC_NPROCESSORS_ONLN));
#endif

#else //WIN32

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;

#endif

#else
    return 1;
#endif //ENABLE_THREADS
}
