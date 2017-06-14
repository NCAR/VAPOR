
#ifndef	_EasyThreads_h_
#define	_EasyThreads_h_

#include <vector>

#ifndef WIN32
#include <pthread.h>
#else
#include <windows.h>
#include <process.h>
#endif
#include "MyBase.h"

namespace Wasp {

class COMMON_API EasyThreads : public MyBase {

public:


 EasyThreads(int nthreads);
 ~EasyThreads();
 int	ParRun(void *(*start)(void *), std::vector <void *> arg);
 int	ParRun(void *(*start)(void *), void **arg);
 int	Barrier();
 int	MutexLock();
 int	MutexUnlock();
 static void	Decompose(int n, int size, int rank, int *offset, int *length);
 static int	NProc();
 int	GetNumThreads() const {return(nthreads_c); }

private:

#ifndef WIN32

 int	nthreads_c;
 pthread_t	*threads_c;
 pthread_attr_t	attr_c;
 pthread_cond_t	cond_c;
 pthread_mutex_t	barrier_lock_c;
 pthread_mutex_t	mutex_lock_c;
 int	block_c;
 int	count_c;	// counters for barrier

#else

 bool initialized_c;
 int nblocked_c;
 int nthreads_c;
 HANDLE* threads_c;
 HANDLE* mutices_c;
 HANDLE* bMutices_c;
 HANDLE mutex_c;
 HANDLE bMutex_c;

#endif
};

};

#endif
