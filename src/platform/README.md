The directory contains basic, default implementation of `CoopThreads` library
callbacks for various platforms. The implementation serves as an example, which
should be sufficient for most of the needs. Every of the callbacks may be
overridden by defining appropriate configuration parameter to provide custom
version of the callback.

For example if there is a need to provide low-power sleep modes support during
the system idle state, a library user may require to:

* Provide custom implementation of `coop_idle_cb()` (`CONFIG_IDLE_CB_ALT` defined)
  to switch the platform into a desired sleep mode.

* Provide custom implementation of `coop_tick_cb()` (`CONFIG_TICK_CB_ALT` defined)
  to adjust the clock ticks with the time spent during the sleep mode.
