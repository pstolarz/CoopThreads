/*
 * Copyright (c) 2020 Piotr Stolarz
 * Lightweight cooperative threads library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

/*
 * UNIX platform specific callbacks implementation.
 */

#ifdef __unix__
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "coop_threads.h"

#if defined(COOP_DEBUG) && !defined(CONFIG_DBG_LOG_CB_ALT)
/**
 * Debug message log callback.
 */
void coop_dbg_log_cb(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#endif

#ifndef CONFIG_TICK_CB_ALT
/**
 * Get clock tick callback (msecs).
 */
coop_tick_t coop_tick_cb()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (coop_tick_t)(tp.tv_sec * 1000LU + tp.tv_nsec / 1000000LU);
}
#endif

#ifndef CONFIG_IDLE_CB_ALT
/**
 * System idle callback.
 */
void coop_idle_cb(coop_tick_t period)
{
    /* ticks in msecs */
    usleep((useconds_t)period * 1000U);
}
#endif
#endif /* __unix__ */
