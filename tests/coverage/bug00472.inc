<?php
/*
 * @author       Elizabeth Smith <auroraeosrose@php.net>
 * @copyright    Elizabeth Smith (c) 2009
 */
namespace Baalat;
abstract class pObject {
    public function emit($name) {
        if (is_null($this->state)) {
            $this->state = new \SplObjectStorage();
        }

        $signal = self::getSignal($name);
        if (is_null($signal)) {
            throw new \InvalidArgumentException('Signal ' . $name . ' is not a registered signal of  ' . get_class($this));
        }

        if (isset($this->state[$signal]) && $this->state[$signal] === self::EMISSION_RUN) {
            $this->state[$signal] = self::EMISSION_RESTART;
            return;
        }

        if (isset(self::$blockedSignals[$signal->class])&& isset(self::$blockedSignals[$signal->class][$name])
            && self::$blockedSignals[$signal->class][$name] > 0) {
            $this->state[$signal] = self::EMISSION_STOP;
            return;
        }

        if (isset($this->blocked[$name]) && $this->blocked[$name] > 0) {
            $this->state[$signal] = self::EMISSION_STOP;
            return;
        }

        $args = func_get_args();
        array_shift($args);

        emit_restart:
            $this->state[$signal] = self::EMISSION_RUN;

            if ($signal->handler && $signal->run === Signal::RUN_FIRST) {
                $run = call_user_func_array(array($this, $signal->handler), $args);
                if ($this->state[$signal] === self::EMISSION_STOP) {
                    goto emit_cleanup;
                }
                if ($this->state[$signal] === self::EMISSION_RESTART) {
                    goto emit_restart;
                }
            }

            if (isset(self::$hooks[$signal->class]) && isset(self::$hooks[$signal->class][$name])) {
                foreach(self::$hooks[$signal->class][$name] as $handler) {
                    $run = call_user_func_array($handler['handler'], $args + $handler['args']);

                    if ((!$signal->accumulator && $run === true) ||
                        ($signal->accumulator && call_user_func(array($this, $signal->accumulator), $run) === true)) {
                        goto emit_cleanup;
                    } else if ($this->state[$signal] === self::EMISSION_STOP) {
                        goto emit_cleanup;
                    } else if ($this->state[$signal] === self::EMISSION_RESTART) {
                        goto emit_restart;
                    }
                }
            }

            if (isset($this->handlers[$name])) {
                foreach($this->handlers[$name] as $handler) {
                    if (!$handler['runlast'] && $handler['blocked'] < 1) {
                        $run = call_user_func_array($handler['handler'], $args + $handler['args']);

                        if ((!$signal->accumulator && $run === true) ||
                            ($signal->accumulator && call_user_func(array($this, $signal->accumulator), $run) === true)) {
                            goto emit_cleanup;
                        } else if ($this->state[$signal] === self::EMISSION_STOP) {
                            goto emit_cleanup;
                        } else if ($this->state[$signal] === self::EMISSION_RESTART) {
                            goto emit_restart;
                        }
                    }
                }
            }

            if ($signal->handler && $signal->run === Signal::RUN_LAST) {
                $run = call_user_func_array(array($this, $signal->handler), $args);
                if ($this->state[$signal] === self::EMISSION_STOP) {
                    goto emit_cleanup;
                }
                if ($this->state[$signal] === self::EMISSION_RESTART) {
                    goto emit_restart;
                }
 
            }

            if (isset($this->handlers[$name])) {
                foreach($this->handlers[$name] as $handler) {
                    if ($handler['runlast'] && $handler['blocked'] < 1) {
                        $run = call_user_func_array($handler['handler'], $args + $handler['args']);

                        if ((!$signal->accumulator && $run === true) ||
                            ($signal->accumulator && call_user_func(array($this, $signal->accumulator), $run) === true)) {
                            goto emit_cleanup;
                        } else if ($this->state[$signal] === self::EMISSION_STOP) {
                            goto emit_cleanup;
                        } else if ($this->state[$signal] === self::EMISSION_RESTART) {
                            goto emit_restart;
                        }
                    }
                }
            }

        emit_cleanup:

            if ($signal->handler && $signal->run === Signal::RUN_CLEANUP) {
                $run = call_user_func_array(array($this, $signal->handler), $args);
            }
            if ($this->state[$signal] === self::EMISSION_RESTART) {
                goto emit_restart;
            }
            $this->state[$signal] = self::EMISSION_STOP;
            return;
    }
}
