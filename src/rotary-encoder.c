/**************************************************************************/ /**
                                                                              *
                                                                              * @file rotary-encoder.c
                                                                              *
                                                                              * @author NOAH BUSTARD
                                                                              * @author CADEN FRANCE
                                                                              *
                                                                              * @brief Code to determine the direction that a rotary encoder is turning.
                                                                              *
                                                                              ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "interrupt_support.h"
#include "rotary-encoder.h"

#define A_WIPER_PIN (16)
#define B_WIPER_PIN (A_WIPER_PIN + 1)

typedef enum
{
    HIGH_HIGH,
    HIGH_LOW,
    LOW_LOW,
    LOW_HIGH,
    UNKNOWN
} rotation_state_t;

static rotation_state_t volatile state;
static direction_t volatile direction = STATIONARY;
static int volatile clockwise_count = 0;
static int volatile counterclockwise_count = 0;

static void handle_quadrature_interrupt();

void initialize_rotary_encoder()
{
    cowpi_set_pullup_input_pins((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN));
    uint8_t quadrature = get_quadrature();
    switch (quadrature)
    {
    case 0b11:
        state = HIGH_HIGH;
        break;
    case 0b10:
        state = HIGH_LOW;
        break;
    case 0b00:
        state = LOW_LOW;
        break;
    case 0b01:
        state = LOW_HIGH;
        break;
    default:
        state = UNKNOWN;
        break;
    }

    register_pin_ISR((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN), handle_quadrature_interrupt);
}

uint8_t get_quadrature()
{
    int a = digitalRead(A_WIPER_PIN);
    int b = digitalRead(B_WIPER_PIN);
    return (b << 1) | a;
}

char *count_rotations(char *buffer)
{
    sprintf(buffer, "CW:%d CCW:%d", clockwise_count, counterclockwise_count);
    return buffer;
}

direction_t get_direction()
{
    direction_t direction_copy = direction;
    direction = STATIONARY;
    return direction_copy;
}

rotation_state_t decode_quadrature(uint8_t quadrature)
{
    rotation_state_t decoded_state = UNKNOWN;
    switch (quadrature)
    {
    case 0b00:
        decoded_state = LOW_LOW;
        break;
    case 0b01:
        decoded_state = LOW_HIGH;
        break;
    case 0b10:
        decoded_state = HIGH_LOW;
        break;
    case 0b11:
        decoded_state = HIGH_HIGH;
        break;
    }
    return decoded_state;
}

static void handle_quadrature_interrupt()
{
    static rotation_state_t last_state = HIGH_HIGH;
    uint8_t quadrature = get_quadrature();
    rotation_state_t decoded_state = decode_quadrature(quadrature);
    rotation_state_t new_state = state;

    if (decoded_state == state)
    {
        return;
    }

    switch (state)
    {
    case LOW_LOW:
        if (decoded_state == HIGH_LOW && last_state == LOW_HIGH)
        {
            new_state = HIGH_LOW;
        }
        else if (decoded_state == LOW_HIGH && last_state == HIGH_LOW)
        {
            new_state = LOW_HIGH;
        }
        break;
    case LOW_HIGH:
        if (decoded_state == HIGH_HIGH)
        {
            new_state = HIGH_HIGH;
        }
        else if (decoded_state == LOW_LOW && last_state == HIGH_HIGH)
        {
            new_state = LOW_LOW;
            counterclockwise_count++;
            direction = COUNTERCLOCKWISE;
        }
        break;
    case HIGH_LOW:
        if (decoded_state == HIGH_HIGH)
        {
            new_state = HIGH_HIGH;
        }
        else if (decoded_state == LOW_LOW && last_state == HIGH_HIGH)
        {
            new_state = LOW_LOW;
            clockwise_count++;
            direction = CLOCKWISE;
        }
        break;
    case HIGH_HIGH:
        if (decoded_state == LOW_HIGH)
        {
            new_state = LOW_HIGH;
        }
        else if (decoded_state == HIGH_LOW)
        {
            new_state = HIGH_LOW;
        }
        break;
    case UNKNOWN:
        new_state = decoded_state;
        break;
    }

    if (new_state != state)
    {
        last_state = state;
        state = new_state;
    }
}