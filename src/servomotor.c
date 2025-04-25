/**************************************************************************//**
 *
 * @file rotary-encoder.c
 *
 * @author Noah Bustard
 * @author Caden France
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

#define SERVO_PIN           (22)
#define PULSE_INCREMENT_uS  (500)
#define SIGNAL_PERIOD_uS    (20000)

static int volatile pulse_width_us;
static int volatile time_until_rising_edge;
static int volatile time_until_falling_edge;
static bool initialized = false;
static bool toggled = false;
static bool button_already_pressed = false;
static bool centered = false;

static void handle_timer_interrupt() {
    time_until_rising_edge -= PULSE_INCREMENT_uS;
    time_until_falling_edge -= PULSE_INCREMENT_uS;

    if (time_until_rising_edge <= 0) {
        digitalWrite(SERVO_PIN, 1);
        time_until_rising_edge += SIGNAL_PERIOD_uS;
        time_until_falling_edge = pulse_width_us;
    }

    if (time_until_falling_edge <= 0) {
        digitalWrite(SERVO_PIN, 0);
    }
}

void initialize_servo() {
    cowpi_set_output_pins(1 << SERVO_PIN);
    center_servo();
    register_periodic_timer_ISR(0, PULSE_INCREMENT_uS, handle_timer_interrupt);
}

char *test_servo(char *buffer) {

    if (!initialized) {
        center_servo();
        initialized = true;
        centered = true;
        sprintf(buffer, "Center");
        return buffer;
    }

    if (cowpi_right_button_is_pressed()) {
        if (!button_already_pressed) {
            toggled = !toggled;
            button_already_pressed = true;
            time_until_rising_edge = 0;
            centered = false;
        }
    } else {
        button_already_pressed = false;
    }

    if (centered) {
        center_servo();
        sprintf(buffer, "Center");
    } else if (toggled) {
        rotate_full_clockwise();
        sprintf(buffer, "Clockwise");
    } else {
        rotate_full_counterclockwise();
        sprintf(buffer, "Counterclockwise");
    }

    return buffer;
}


void center_servo() {
    pulse_width_us = 1500;
}

void rotate_full_clockwise() {
    pulse_width_us = 1000;
}

void rotate_full_counterclockwise() {
    pulse_width_us = 2000;
}
