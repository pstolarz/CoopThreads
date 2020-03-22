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

/*
 * Arduino platform specific callbacks implementation.
 */

#ifdef ARDUINO
#include <Arduino.h>
#include "coop_threads.h"

extern "C" {

#if defined(COOP_DEBUG) && !defined(CONFIG_DBG_LOG_CB_ALT)
/**
 * Debug message log callback.
 */
void coop_dbg_log_cb(const char *format, ...)
{
    static char msg[128] = {};
    va_list args;

    va_start(args, format);
    vsprintf(msg, format, args);
    va_end(args);

    Serial.print(msg);
}

#ifdef ARDUINO_ARCH_AVR
/**
 * Get stack pointer (AVR only).
 */
uint16_t get_sp(void) {
    return (((uint16_t)SPH << 8) | (uint16_t)SPL);
}
#endif
#endif

#ifndef CONFIG_TICK_CB_ALT
/**
 * Get clock tick callback (msecs).
 */
coop_tick_t coop_tick_cb()
{
    return millis();
}
#endif

#ifndef CONFIG_IDLE_CB_ALT
/**
 * System idle callback.
 */
void coop_idle_cb(coop_tick_t period)
{
    delay(period);
}
#endif

} /* extern "C" */
#endif /* ARDUINO */
