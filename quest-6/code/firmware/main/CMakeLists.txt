set(COMPONENT_SRCS
    "wifi.c"
    "alphanumeric_display.c"
    "crawler_control.c"
    "pulse_counter.c"
    "pid_control.c"
    "lidar.c"
    "lidarlite.c"
    "ultrasonic.c"
    "timer.c"
    "server_communication.c"
    "crawler_main.c")

set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
