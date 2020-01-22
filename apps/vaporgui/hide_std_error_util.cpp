#include "hide_std_error_util.h"

#ifndef WIN32
#include <unistd.h>
#include <stdio.h>
#endif

static int _savedSTDERR;
static fpos_t pos;

void HideSTDERR()
{
#ifndef WIN32
    fflush( stderr );
    fgetpos( stderr, &pos );

    _savedSTDERR = dup(STDERR_FILENO);

    freopen( "/dev/null", "w", stderr );
#endif
}
void RestoreSTDERR()
{
#ifndef WIN32
    fflush( stderr );

    dup2(  _savedSTDERR, STDERR_FILENO );
    close( _savedSTDERR );

    clearerr( stderr );
    fsetpos( stderr, &pos );
#endif
}
