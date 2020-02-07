#include "hide_std_error_util.h"

#ifndef WIN32
#include <unistd.h>
#include <stdio.h>
#endif

static int _savedSTDERR;

void HideSTDERR()
{
#ifndef WIN32
    _savedSTDERR = -1;
    if ( fflush( stderr ) != 0 )
        return;

    int rc = dup(STDERR_FILENO);
    if (rc < 0)
        return;
    else
        _savedSTDERR = rc;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    freopen( "/dev/null", "w", stderr );
#pragma GCC diagnostic pop
#endif
}
void RestoreSTDERR()
{
#ifndef WIN32
    if ( _savedSTDERR == -1 )
        return;

    fflush( stderr );

    dup2(  _savedSTDERR, STDERR_FILENO );
    close( _savedSTDERR );

    clearerr( stderr );
#endif
}
