//
// $Id$
//

// pivotal_gmtime_r - a replacement for gmtime/localtime/mktime
//                   that works around the 2038 bug on 32-bit
//                    systems. (Version 3)
//
// Copyright (C) 2005  Paul Sheer
//
// Redistribution and use in source form, with or without modification,
// is permitted provided that the above copyright notice, this list of
// conditions, the following disclaimer, and the following char array
// are retained.
//
// Redistribution and use in binary form must reproduce an
//
// acknowledgment: 'With software provided by http://2038bug.com/' in
// the documentation and/or other materials provided with the
// distribution, and wherever such acknowledgments are usually
// accessible in Your program.
//
// This software is provided "AS IS" and WITHOUT WARRANTY, either
// express or implied, including, without limitation, the warranties of
// NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. THE ENTIRE RISK AS TO THE QUALITY OF THIS SOFTWARE IS WITH
// YOU. Under no circumstances and under no legal theory, whether in
// tort (including negligence), contract, or otherwise, shall the
// copyright owners be liable for any direct, indirect, special,
// incidental, or consequential damages of any character arising as a
// result of the use of this software including, without limitation,
// damages for loss of goodwill, work stoppage, computer failure or
// malfunction, or any and all other commercial damages or losses. This
// limitation of liability shall not apply to liability for death or
// personal injury resulting from copyright owners' negligence to the
// extent applicable law prohibits such limitation. Some jurisdictions
// do not allow the exclusion or limitation of incidental or
// consequential damages, so this exclusion and limitation may not apply
// to You.


#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <ctime>
#include <vapor/PVTime.h>

using namespace Wasp;

// mktime64() is a 64-bit equivalent of mktime().
// localtime64_r() is a 64-bit equivalent of localtime_r().
// gmtime64_r() is a 64-bit equivalent of gmtime_r().


static const int days[4][13] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};

#define LEAP_CHECK(n)	((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
#define WRAP(a,b,m)	((a) = ((a) <  0  ) ? ((b)--, (a) + (m)) : (a))


TIME64_T pivot_time_t (const time_t * now, TIME64_T *_t)
{
    TIME64_T t;
    t = *_t;
    if (now && sizeof (time_t) == 4) {
        time_t _now;
        _now = *now;
        if (_now < 1106500000 /* Jan 23 2005 - date of writing */ )
            _now = 2147483647;
        if ((TIME64_T) t + ((TIME64_T) 1 << 31) < (TIME64_T) _now)
            t += (TIME64_T) 1 << 32;
    }
    return t;
}

static struct tm *_gmtime64_r (const time_t * now, TIME64_T *_t, struct tm *p)
{
    int v_tm_sec, v_tm_min, v_tm_hour, v_tm_mon, v_tm_wday, v_tm_tday;
    int leap;
    TIME64_T t;
    long m;
    t = pivot_time_t (now, _t);
    v_tm_sec = ((TIME64_T) t % (TIME64_T) 60);
    t /= 60;
    v_tm_min = ((TIME64_T) t % (TIME64_T) 60);
    t /= 60;
    v_tm_hour = ((TIME64_T) t % (TIME64_T) 24);
    t /= 24;
    v_tm_tday = t;
    WRAP (v_tm_sec, v_tm_min, 60);
    WRAP (v_tm_min, v_tm_hour, 60);
    WRAP (v_tm_hour, v_tm_tday, 24);
    if ((v_tm_wday = (v_tm_tday + 4) % 7) < 0)
        v_tm_wday += 7;
    m = (long) v_tm_tday;
    if (m >= 0) {
        p->tm_year = 70;
        leap = LEAP_CHECK (p->tm_year);
        while (m >= (long) days[leap + 2][12]) {
            m -= (long) days[leap + 2][12];
            p->tm_year++;
            leap = LEAP_CHECK (p->tm_year);
        }
        v_tm_mon = 0;
        while (m >= (long) days[leap][v_tm_mon]) {
            m -= (long) days[leap][v_tm_mon];
            v_tm_mon++;
        }
    } else {
        p->tm_year = 69;
        leap = LEAP_CHECK (p->tm_year);
        while (m < (long) -days[leap + 2][12]) {
            m += (long) days[leap + 2][12];
            p->tm_year--;
            leap = LEAP_CHECK (p->tm_year);
        }
        v_tm_mon = 11;
        while (m < (long) -days[leap][v_tm_mon]) {
            m += (long) days[leap][v_tm_mon];
            v_tm_mon--;
        }
        m += (long) days[leap][v_tm_mon];
    }
    p->tm_mday = (int) m + 1;
    p->tm_yday = days[leap + 2][v_tm_mon] + m;
    p->tm_sec = v_tm_sec, p->tm_min = v_tm_min, p->tm_hour = v_tm_hour,
        p->tm_mon = v_tm_mon, p->tm_wday = v_tm_wday;
    return p;
}

static struct tm *_localtime64_r (const time_t * now, TIME64_T *_t, struct tm *p)
{
    TIME64_T tl;
    time_t t;
    struct tm tm, tm_localtime, tm_gmtime;
    _gmtime64_r (now, _t, &tm);
    if (tm.tm_year > (2037 - 1900))
        tm.tm_year = 2037 - 1900;
    t = MkTime64 (&tm);
#ifdef	WIN32
#pragma warning( disable : 4996 )
	struct tm *tm_localtime_ptr, *tm_gmtime_ptr;

    tm_localtime_ptr = localtime (&t);
	tm_localtime = *tm_localtime_ptr;

    tm_gmtime_ptr = gmtime (&t);
	tm_gmtime = *tm_gmtime_ptr;
#else
    localtime_r (&t, &tm_localtime);
    gmtime_r (&t, &tm_gmtime);
#endif
    tl = *_t;
    tl += (MkTime64 (&tm_localtime) - MkTime64 (&tm_gmtime));
    _gmtime64_r (now, &tl, p);
    p->tm_isdst = tm_localtime.tm_isdst;
    return p;
}

struct tm *Wasp::GmTime64_r (const TIME64_T *_t, struct tm *p)
{
    TIME64_T t;
    t = *_t;
    return _gmtime64_r (NULL, &t, p);
}

TIME64_T Wasp::MkTime64 (struct tm *t)
{
    int i, y;
    long day = 0;
    TIME64_T r;
    if (t->tm_year < 70) {
        y = 69;
        do {
            day -= 365 + LEAP_CHECK (y);
            y--;
        } while (y >= t->tm_year);
    } else {
        y = 70;
        while (y < t->tm_year) {
            day += 365 + LEAP_CHECK (y);
            y++;
        }
    }
    for (i = 0; i < t->tm_mon; i++)
        day += days[LEAP_CHECK (t->tm_year)][i];
    day += t->tm_mday - 1;
    t->tm_wday = (int) ((day + 4) % 7);
    r = (TIME64_T) day *86400;
    r += t->tm_hour * 3600;
    r += t->tm_min * 60;
    r += t->tm_sec;
    return r;
}



struct tm *Wasp::LocalTime64_r (const TIME64_T *_t, struct tm *p)
{
    TIME64_T tl;
    tl = *_t;
    return _localtime64_r (NULL, &tl, p);
}
