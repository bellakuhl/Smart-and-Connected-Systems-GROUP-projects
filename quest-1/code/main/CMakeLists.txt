set(COMPONENT_SRCS 
                    "lib/alphanumeric_display.c"
                    "lib/console_io.c"
                    "lib/servo.c"
                    "retro_clock_display.c"
                    "retro_clock_io.c"
                    "retro_clock_hands.c"
                    "retro_clock_timing.c"
                    "retro_clock_main.c")
    set(COMPONENT_ADD_INCLUDEDIRS "")


register_component()
