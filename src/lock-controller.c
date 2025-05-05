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
#define CHANGING (3)

static uint8_t combination[3] __attribute__((section(".uninitialized_ram.")));
static uint8_t entered_combination[3];
static uint8_t lock_state;
static uint8_t progress;
static uint8_t bad_tries;
static uint8_t times_passed_first_num;
static uint8_t times_passed_second_num;
static uint8_t times_passed_third_num;
static uint8_t times_passed_zero;

uint8_t const *get_combination()
{
    return combination;
}

void force_combination_reset()
{
    combination[0] = 05;
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
    times_passed_first_num = 0;
    times_passed_second_num = 0;
    times_passed_third_num = 0;
    times_passed_zero = 0;
    rotate_full_clockwise();
    cowpi_illuminate_left_led();
    display_string(1, format_combination(entered_combination));
}

void control_lock()
{

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
                if (progress == 2 && times_passed_zero == 2)
                {
                    times_passed_first_num = 0;
                    times_passed_second_num = 0;
                    times_passed_third_num = 0;
                    times_passed_zero = 0;
                    entered_combination[0] = 255;
                    entered_combination[1] = 255;
                    entered_combination[2] = 255;
                    progress = 0;
                }
                if (entered_combination[progress] == combination[progress])
                {
                    if (progress == 0)
                    {
                        times_passed_first_num += 1;
                    }
                    else
                    {
                        times_passed_third_num += 1;
                    }
                }
                entered_combination[progress] = (entered_combination[progress] + 1) % 16;
                if (entered_combination[progress] == 0 && progress == 2)
                {
                    times_passed_zero += 1;
                }
            }
            else if (progress == 1)
            {
                progress = 2;
                times_passed_zero = 0;
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
                if (times_passed_zero == 3)
                {
                    times_passed_first_num = 0;
                    times_passed_second_num = 0;
                    times_passed_third_num = 0;
                    times_passed_zero = 0;
                    entered_combination[0] = 255;
                    entered_combination[1] = 255;
                    entered_combination[2] = 255;
                    progress = 0;
                }
                else if (entered_combination[progress] == combination[progress])
                {
                    times_passed_second_num += 1;
                }
                entered_combination[progress] = (entered_combination[progress] - 1 + 16) % 16;
                if (entered_combination[progress] == 0)
                {
                    times_passed_zero += 1;
                }
            }
            else if (progress == 2)
            {
                times_passed_first_num = 0;
                times_passed_second_num = 0;
                times_passed_third_num = 0;
                times_passed_zero = 0;
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
                    entered_combination[2] == combination[2] &&
                    times_passed_first_num > 1 &&
                    times_passed_second_num == 1 &&
                    times_passed_third_num == 0)
                {
                    lock_state = UNLOCKED;
                    rotate_full_counterclockwise();
                    cowpi_illuminate_right_led();
                    cowpi_deluminate_left_led();
                    display_string(1, "OPEN");
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
                        volatile unsigned long j;
                        char bad_try_message[16];
                        sprintf(bad_try_message, "bad try %d\n", bad_tries);
                        display_string(1, bad_try_message);

                        for (int i = 0; i < 2; i++)
                        {
                            display_string(1, bad_try_message);
                            cowpi_illuminate_left_led();
                            cowpi_illuminate_right_led();
                            for (j = 0; j < 2000000; j++)
                            {
                            }
                            display_string(1, bad_try_message);
                            cowpi_deluminate_left_led();
                            cowpi_deluminate_right_led();
                            for (j = 0; j < 2000000; j++)
                            {
                            }
                        }
                        entered_combination[0] = 255;
                        entered_combination[1] = 255;
                        entered_combination[2] = 255;
                        times_passed_first_num = 0;
                        times_passed_second_num = 0;
                        times_passed_third_num = 0;
                        times_passed_zero = 0;
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
            times_passed_first_num = 0;
            times_passed_second_num = 0;
            times_passed_third_num = 0;
            times_passed_zero = 0;
            display_string(1, format_combination(entered_combination));
            lock_state = LOCKED;
        }
        if (cowpi_left_switch_is_in_right_position() && right_button_pressed)
        {
            lock_state = CHANGING;
        }
    }
    else if (lock_state == ALARMED)
    {
        display_string(1, "ALERT");
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
    else if (lock_state == CHANGING)
    {
        static uint8_t new_combination[6];
        static int index = 0;
        static bool confirming = false;

        char key = cowpi_get_keypress();

        if (!confirming)
        {
            char combo_string[9];
            combo_string[0] = '0' + new_combination[0];
            combo_string[1] = '0' + new_combination[1];
            combo_string[2] = '-';
            combo_string[3] = '0' + new_combination[2];
            combo_string[4] = '0' + new_combination[3];
            combo_string[5] = '-';
            combo_string[6] = '0' + new_combination[4];
            combo_string[7] = '0' + new_combination[5];
            combo_string[8] = '\0';

            display_string(1, "ENTER");
            display_string(2, combo_string);
            if (key >= '0' && key <= '9')
            {
                new_combination[index++] = key - '0';
                if (index == 6)
                {
                    display_string(1, "CONFIRM");
                    confirming = true;
                    index = 0;
                }
            }
        }
        else
        {
            if (key >= '0' && key <= '9')
            {
                if (index < 6 && new_combination[index] == (key - '0'))
                {
                    index++;
                }
                else
                {
                    index = 6;
                }
            }

            if (index == 6 || cowpi_left_switch_is_in_left_position())
            {
                bool incomplete = index < 6;
                bool invalid = false;

                // Check if any number exceeds 15
                for (int i = 0; i < 6; i += 2)
                {
                    int number = new_combination[i] * 10 + new_combination[i + 1];
                    if (number > 15)
                    {
                        invalid = true;
                    }
                }

                if (incomplete || invalid)
                {
                    display_string(1, "NO CHANGE");
                }
                else
                {
                    combination[0] = new_combination[0] * 10 + new_combination[1];
                    combination[1] = new_combination[2] * 10 + new_combination[3];
                    combination[2] = new_combination[4] * 10 + new_combination[5];
                    display_string(1, "CHANGED");
                }

                confirming = false;
                index = 0;
                lock_state = UNLOCKED;
            }
        }
    }
}
