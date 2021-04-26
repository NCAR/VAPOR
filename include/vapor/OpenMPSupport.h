#pragma once

#ifdef USE_OMP
    #include <omp.h>
#else

int omp_get_num_threads() { return (1); }

void omp_set_num_threads(int) {}

int omp_get_thread_num() { return (0); }

#endif
