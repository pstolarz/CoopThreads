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

#ifndef __COOP_THREADS_CONFIG_H__
#define __COOP_THREADS_CONFIG_H__

#ifdef COOP_THREADS_CONFIG_FILE
/* use user defined config file instead of this one */
# include COOP_THREADS_CONFIG_FILE
#elif !defined(COOP_THREADS_DISABLE_DEFAULT_CONFIG)

/**
 * Default thread stack size.
 */
#define CONFIG_DEFAULT_STACK_SIZE 0x100

/**
 * Maximum number of threads supported by the library.
 * Defined as threads pool size.
 */
#define CONFIG_MAX_THREADS 5

/**
 * Alternative implementation of @c coop_dbg_log_cb() callback implementation
 * used for logging debug messages.
 *
 * Debug messages are logged if the library is compiled with @c __DEBUG__
 * macro-define by @c coop_dbg_log_cb(const char *format, ...) routine, which
 * by default is an alias to @c printf(3). By defining @c CONFIG_DBG_LOG_CB_ALT
 * there is possible to provide custom implementation for the logging routine
 * using the following code snippet in the user code:
 *
 * @code
 *     #ifdef __DEBUG__
 *
 *     #ifdef __cplusplus
 *     extern "C" {
 *     #endif
 *
 *     void coop_dbg_log_cb(const char *format, ...)
 *     {
 *         // custom implementation goes here
 *     }
 *
 *     #ifdef __cplusplus
 *     }
 *     #endif
 *
 *     #endif
 * @endcode
 */
//#define CONFIG_DBG_LOG_CB_ALT

#endif /* !COOP_THREADS_DISABLE_DEFAULT_CONFIG */
#endif /* __COOP_THREADS_CONFIG_H__ */
