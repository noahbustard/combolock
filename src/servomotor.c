/**************************************************************************/ /**
                                                                             *
                                                                              * @file rotary-encoder.c
                                                                              *
                                                                              * @author NOAH BUSTARD
                                                                              * @author CADEN FRANCE
                                                                              *
                                                                              * @brief Code to control a servomotor.
                                                                              *
                                                                              ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "servomotor.h"
#include "interrupt_support.h"

#define SERVO_PIN (22)
#define PULSE_INCREMENT_uS (500)
#define SIGNAL_PERIOD_uS (20000)

static int volatile pulse_width_us;
static int volatile time_until_rising_edge_us;
static int volatile time_until_falling_edge_us;

static void handle_timer_interrupt();

void initialize_servo()
{
    cowpi_set_output_pins(1 << SERVO_PIN);
    center_servo();
    time_until_rising_edge_us = 0;
    time_until_falling_edge_us = -1;
    register_periodic_timer_ISR(0, PULSE_INCREMENT_uS, handle_timer_interrupt);
}

char *test_servo(char *buffer)
{
    if (cowpi_left_button_is_pressed())
    {
        center_servo();
        sprintf(buffer, "Center");
    }
    else
    {
        if (cowpi_left_switch_is_in_left_position())
        {
            rotate_full_clockwise();
            sprintf(buffer, "Clockwise");
        }
        else if (cowpi_left_switch_is_in_right_position())
        {
            rotate_full_counterclockwise();
            sprintf(buffer, "Counterclockwise");
        }
    }

    return buffer;
}

void center_servo()
{
    pulse_width_us = 3 * PULSE_INCREMENT_uS;
}

void rotate_full_clockwise()
{
    pulse_width_us = PULSE_INCREMENT_uS;
}

void rotate_full_counterclockwise()
{
    pulse_width_us = 5 * PULSE_INCREMENT_uS;
}

static void handle_timer_interrupt()
{
    if (time_until_rising_edge_us <= 0)
    {
        digitalWrite(SERVO_PIN, 1);
        time_until_rising_edge_us += SIGNAL_PERIOD_uS;
        time_until_falling_edge_us = pulse_width_us;
    }

    if (time_until_falling_edge_us <= 0)
    {
        digitalWrite(SERVO_PIN, 0);
    }

    time_until_rising_edge_us -= PULSE_INCREMENT_uS;
    time_until_falling_edge_us -= PULSE_INCREMENT_uS;
}
