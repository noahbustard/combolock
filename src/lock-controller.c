/**************************************************************************/ /**
                                                                              *
                                                                              * @file lock-controller.c
                                                                              *
                                                                              * @author Noah Bustard
                                                                              * @author Caden France
                                                                              *
                                                                              * @brief Code to implement the "combination lock" mode.
                                                                              *
                                                                              ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "display.h"
#include "lock-controller.h"
#include "rotary-encoder.h"
#include "servomotor.h"
#define LOCKED (1)
#define UNLOCKED (0)
#define ALARMED (2)

static uint8_t combination[3] __attribute__((section(".uninitialized_ram.")));
static uint8_t entered_combination[3];
static uint8_t lock_state;
static uint8_t progress;
static uint8_t bad_tries;

uint8_t const *get_combination()
{
    return combination;
}

void force_combination_reset()
{
    combination[0] = 5;
    combination[1] = 10;
    combination[2] = 15;
}

const char *format_combination(uint8_t combination[3])
{
    static char formatted_combination[12];
    char first[3] = "  ";
    char second[3] = "  ";
    char third[3] = "  ";
    if (combination[0] <= 15)
    {
        sprintf(first, "%02d", combination[0]);
    }
    if (combination[1] <= 15)
    {
        sprintf(second, "%02d", combination[1]);
    }
    if (combination[2] <= 15)
    {
        sprintf(third, "%02d", combination[2]);
    }

    sprintf(formatted_combination, "%s-%s-%s", first, second, third);
    return formatted_combination;
}

void initialize_lock_controller()
{
    force_combination_reset();
    entered_combination[0] = 255;
    entered_combination[1] = 255;
    entered_combination[2] = 255;
    lock_state = LOCKED;
    progress = 0;
    bad_tries = 0;
    rotate_full_clockwise();
    cowpi_illuminate_left_led();
    display_string(1, format_combination(entered_combination));
}

// returns the number of times a target number has been availible for selection by the lock at this position
int compute_times_seen(int start, int target, bool clockwise, int rotations)
{
    int start_to_target;
    int times_seen;
    if (clockwise)
    {
        if (target >= start)
        {
            start_to_target = target - start;
        }
        else
        {
            start_to_target = 16 - (start - target);
        }
    }
    else
    {
        if (start >= target)
        {
            start_to_target = start - target;
        }
        else
        {
            start_to_target = 16 - (target - start);
        }
    }
    if (rotations >= start_to_target)
    {
        times_seen++;
        rotations -= start_to_target;
    }
    while (rotations >= 16)
    {
        times_seen++;
        rotations -= 16;
    }
}

void control_lock()
{

    // TODO: Add way to track if numbers have been passed already PDF Section 6 Requirement 13
    int direction = get_direction();
    bool left_button_pressed = cowpi_debounce_byte(cowpi_left_button_is_pressed(), LEFT_BUTTON_DOWN);
    bool right_button_pressed = cowpi_debounce_byte(cowpi_right_button_is_pressed(), RIGHT_BUTTON_DOWN);
    bool both_buttons_pressed = left_button_pressed && right_button_pressed;
    if (lock_state == LOCKED)
    {
        if (direction == CLOCKWISE)
        {
            if (progress == 0 || progress == 2)
            {
                entered_combination[progress] = (entered_combination[progress] + 1) % 16;
            }
            else if (progress == 1)
            {
                progress = 2;
                entered_combination[2] = entered_combination[1];
            }
        }
        else if (direction == COUNTERCLOCKWISE)
        {
            if (progress == 0)
            {
                progress = 1;
                entered_combination[1] = entered_combination[0];
            }
            else if (progress == 1)
            {
                entered_combination[progress] = (entered_combination[progress] - 1 + 16) % 16;
            }
            else if (progress == 2)
            {
                entered_combination[0] = 255;
                entered_combination[1] = 255;
                entered_combination[2] = 255;
                progress = 0;
            }
        }
        display_string(1, format_combination(entered_combination));
        if (left_button_pressed)
        {
            if (progress == 2)
            {
                if (entered_combination[0] == combination[0] &&
                    entered_combination[1] == combination[1] &&
                    entered_combination[2] == combination[2])
                {
                    lock_state = UNLOCKED;
                    rotate_full_counterclockwise();
                    cowpi_illuminate_right_led();
                    cowpi_deluminate_left_led();
                    display_string(1, "Unlocked!");
                }
                else
                {
                    bad_tries += 1;
                    if (bad_tries >= 3)
                    {
                        lock_state = ALARMED;
                    }
                    else
                    {
                        char bad_try_message[12];
                        volatile unsigned long j;
                        sprintf(bad_try_message, "bad try %d", bad_tries);
                        display_string(1, bad_try_message);

                        for (int i = 0; i < 2; i++)
                        {
                            cowpi_illuminate_left_led();
                            cowpi_illuminate_right_led();
                            for (j = 0; j < 2000000; j++)
                            {
                            }
                            cowpi_deluminate_left_led();
                            cowpi_deluminate_right_led();
                            for (j = 0; j < 2000000; j++)
                            {
                            }
                        }
                        entered_combination[0] = 255;
                        entered_combination[1] = 255;
                        entered_combination[2] = 255;
                        progress = 0;
                    }
                }
            }
            else
            {
                progress += 1;
            }
        }
    }
    else if (lock_state == UNLOCKED)
    {
        if (both_buttons_pressed)
        {
            rotate_full_clockwise();
            cowpi_illuminate_left_led();
            cowpi_deluminate_right_led();
            entered_combination[0] = 255;
            entered_combination[1] = 255;
            entered_combination[2] = 255;
            progress = 0;
            display_string(1, format_combination(entered_combination));
            lock_state = LOCKED;
        }
    }
    else if (lock_state == ALARMED)
    {

        volatile unsigned long i;
        cowpi_illuminate_left_led();
        cowpi_illuminate_right_led();
        for (i = 0; i < 1000000; i++)
        {
        }
        cowpi_deluminate_left_led();
        cowpi_deluminate_right_led();
        for (i = 0; i < 1000000; i++)
        {
        }
    }
}
