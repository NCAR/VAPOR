#ifndef OPENMPSUPPORT_H
#define OPENMPSUPPORT_H

#ifdef USE_OMP
    #include <omp.h>
#else

#define omp_get_num_threads() (1)
#define omp_set_num_threads(x) (void(x))
#define omp_get_thread_num() (0)

#endif

#endif
