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
 * HAL platform specific callbacks implementation.
 */

#ifdef __STM32_HAL__
/* Include HAL_Driveres via file generated by CubeMX */
#include "main.h"

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
    return HAL_GetTick();
}
#endif

#ifndef CONFIG_IDLE_CB_ALT
/**
 * System idle callback.
 */
void coop_idle_cb(coop_tick_t period)
{
    /* ticks in msecs */
    HAL_Delay(period);
}
#endif
#endif /* __STM32_HAL__ */
