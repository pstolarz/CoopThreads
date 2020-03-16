/*
 * Copyright (c) 2020 Piotr Stolarz
 * Lightweight Cooperative Threads library
 *
 * Distributed under the 2-clause BSD License (the License)
 * see accompanying file LICENSE for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __COOP_THREADS_H__
#define __COOP_THREADS_H__

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __DEBUG__
/* debug turned off */
#define coop_dbg_log_cb(...)
#elif defined(CONFIG_DBG_LOG_CB_ALT)
/* debug callback provided externally */
void coop_dbg_log_cb(const char *format, ...);
#else
/* default debug callback */
# include <stdio.h>
# define coop_dbg_log_cb printf
#endif /* __DEBUG__ */

typedef enum
{
    COOP_SUCCESS = 0,
    COOP_INV_ARG,
    COOP_ERR_LIMIT
} coop_error_t;

typedef void (*coop_thrd_proc_t)(void*);

/**
 * Run scheduler service to run scheduled threads.
 * The routine returns when last scheduled thread ends.
 */
void coop_sched_service(void);

/**
 * Schedule thread to run.
 */
coop_error_t coop_sched_thread(coop_thrd_proc_t proc, const char *name,
    size_t stack_sz, void *arg);

/**
 * Yield currently running thread back to scheduler.
 * To be called from thread routine only.
 */
void coop_sched_yield(void);

/**
 * Get currently running thread name.
 * To be called from thread routine only.
 */
const char *coop_get_thread_name(void);

#ifdef __cplusplus
}
#endif

#endif /* __COOP_THREADS_H__ */
