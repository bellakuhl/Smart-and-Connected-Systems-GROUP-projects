#include "retro_clock.h"
#include "lib/console_io.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/*void retro_clock_change_mode(retro_clock_t *clock, retro_clock_mode_t mode){
	clock->clock_mode = mode;
}*/

bool hour_is_valid(char *hour){
	// keeps asking till it gets a valid hour
	int hour_int = atoi(hour);
	if ( hour_int < 0 || hour_int >= 24){
		return false;
	} else{
		for (int i = 0; i<1024; i++){
			if (!isdigit((int)hour[0])){
				return false;
			}
		}
	}
	return true;
}

bool minute_is_valid(char *minute){
	// keeps asking till it gets a valid hour
	int min_int = atoi(minute);
	if ( min_int < 0 || min_int >= 60){
		return false;
	} else{
		for (int i = 0; i<1024; i++){
			if (!isdigit((int)minute[0])){
				return false;
			}
		}
	}
	return true;
}

void retro_clock_io_set_time(retro_clock_t *clock){
	retro_clock_change_mode(clock, RC_MODE_SET_TIME);

	io_uart_writes("Enter hour: ");
	char hour[1024];
    memset(hour, '\0', 1024);
    io_uart_readline(hour, 1024, 0,true);

	while(1){
		if (!hour_is_valid(hour)){
			io_uart_writes("Invalid input. Please enter an integer from 0-23: ");
			io_uart_readline(hour, 1024, 0,true);
			io_uart_writes("\r\n");
		}else{
			break;
		}

	}
	retro_clock_time_t time = {atoi(hour), 0, 0};
	retro_clock_set_time(clock, time);

    io_uart_writes("\r\n");
	io_uart_writes("Enter minutes: ");
	char minute[1024];
    memset(minute, '\0', 1024);
    io_uart_readline(minute, 1024, 0,true);
    io_uart_writes("\r\n");

	while(1){
		if (!(minute_is_valid(minute))){
			io_uart_writes("Invalid input. Please enter an integer from 0-59: ");
			io_uart_readline(minute, 1024, 0,true);
			io_uart_writes("\r\n");
		}else{
			break;
		}
	}
	retro_clock_time_t ntime = {atoi(hour), atoi(minute), 0};
	retro_clock_set_time(clock, ntime);
	retro_clock_change_mode(clock, RC_MODE_CLOCK);
}

void retro_clock_io_set_alarm(retro_clock_t *clock){
	retro_clock_change_mode(clock, RC_MODE_SET_ALARM);
	io_uart_writes("Enter hour: ");
	char hour0[1024];
    memset(hour0, '\0', 1024);
    io_uart_readline(hour0, 1024, 0,true);
    io_uart_writes("\r\n");


	while(1){
		if (!hour_is_valid(hour0)){
			io_uart_writes("Invalid input. Please enter an integer from 0-23: ");
			io_uart_readline(hour0, 1024, 0,true);
			io_uart_writes("\r\n");
		}else{
			break;
		}

	}

	io_uart_writes("Enter minutes: ");
	char minute[1024];
    memset(minute, '\0', 1024);
    io_uart_readline(minute, 1024, 0,true);
    io_uart_writes("\r\n");

	while(1) {
		if (!(minute_is_valid(minute))){
			io_uart_writes("Invalid input. Please enter an integer from 0-59: ");
			io_uart_readline(minute, 1024, 0,true);
            io_uart_writes("\r\n");
		}else{
			break;
		}
	}
	retro_clock_time_t time = {atoi(hour0), atoi(minute), 0};
	retro_clock_alarm_set_time(clock, time);
	io_uart_writeline("Alarm has been set.");
	retro_clock_change_mode(clock, RC_MODE_CLOCK);
}

void retro_clock_io_init(retro_clock_t *clock)
{
	char *start = "Press s to set the clock time";
	size_t buffer_size = 1024;
	io_uart_setup(buffer_size);
	io_uart_writeline(start);
	char input = io_uart_getc();
	while (!(input == 's')){
		input = io_uart_getc();
	}
    retro_clock_change_mode(clock, RC_MODE_SET_TIME);
}

void retro_clock_enable_alarm(retro_clock_t *clock){
	retro_clock_alarm_set_state(clock, RC_ALARM_STATE_ENABLED);
    io_uart_writeline("Alarm is enabled.");
}

void retro_clock_disable_alarm(retro_clock_t *clock){
	retro_clock_alarm_set_state(clock, RC_ALARM_STATE_NOT_ENABLED);
    io_uart_writeline("Alarm is disabled.");
}

void retro_clock_io_dismiss_alarm(retro_clock_t *clock)
{
	io_uart_writeline("In alarm mode, type 'dismiss': ");
	char input[1024];
    memset(input, '\0', 1024);
    io_uart_readline(input, 1024, 0,true);
    io_uart_writes("\r\n");

	while(1) {
		if (strcmp(input, "dismiss") != 0) {
			io_uart_writes("Invalid input. Please type the word 'dismiss': ");
			io_uart_readline(input, 1024, 0,true);
            io_uart_writes("\r\n");
		}else{
			break;
		}
	}

    retro_clock_alarm_dismiss(clock);
}

void retro_clock_io_main(retro_clock_t *clock)
{
    while (1)
    {
        if (clock->clock_mode == RC_MODE_CLOCK)
        {
            char *prompt = "Your clock is now running. Available commands: \n"
                            "Set clock time - Press 'c'\n"
                            "Set alarm time - Press 'a'\n"
                            "Enable alarm   - Press 'e'\n"
                            "Disable alarm  - Press 'd'";
                            //"Help - Press 'h'";

            io_uart_writeline(prompt);
            // take in user input and call function for input
            char input = io_uart_getc();
            while (strchr("cxaed", input) == NULL){
                io_uart_writes("Invalid input. Please try again: ");
                input = io_uart_getc();
                io_uart_writeline("\r\n");
            }
            if (input == 'c'){
                retro_clock_change_mode(clock, RC_MODE_SET_TIME);
            }else if (input == 'a'){
                retro_clock_change_mode(clock, RC_MODE_SET_ALARM);
            }else if (input == 'e'){
                retro_clock_enable_alarm(clock);
            }else if (input == 'd'){
                retro_clock_disable_alarm(clock);
            }
        }
        else if (clock->clock_mode == RC_MODE_SET_TIME) {
            retro_clock_io_set_time(clock);
            retro_clock_change_mode(clock, RC_MODE_CLOCK);
        }
        else if (clock->clock_mode == RC_MODE_SET_ALARM) {
            retro_clock_io_set_alarm(clock);
            retro_clock_change_mode(clock, RC_MODE_CLOCK);
        }
        else if (clock->clock_mode == RC_MODE_IN_ALARM) {
            retro_clock_io_dismiss_alarm(clock);
        }
    }
}
