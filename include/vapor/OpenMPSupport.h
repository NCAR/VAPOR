
#pragma once

#include "VaporConfig.h"

#ifdef USE_OMP
    #include <omp.h>
#else

int omp_get_num_threads() { return (1); }

int omp_get_thread_num() { return (0); }

#endif
