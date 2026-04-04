#pragma once

inline uint16_t *ledsAddress;
inline uint16_t ledsImage;

inline void updateHardware()
{
    *ledsAddress = ledsImage;
}

enum
{
    ALL_LEDS_ON = ~0,
    ALL_LEDS_OFF = ~ALL_LEDS_ON
};

enum
{
    FIRST_LED = 1,
    LAST_LED = 16
};

inline uint16_t convertLedNumberToBit(int led_number)
{
    return 1 << (led_number - 1);
}

inline void setLedImageBit(int ledNumber)
{
    ledsImage |= convertLedNumberToBit(ledNumber);
}

static void clearLedImageBit(int ledNumber)
{
    ledsImage &= ~convertLedNumberToBit(ledNumber);
}

inline bool IsLedOutOfBounds(int led_number)
{
    return (led_number < FIRST_LED) || (led_number > LAST_LED);
}

inline void LedDriver_Create(uint16_t *address)
{
    ledsAddress = address;
    ledsImage = ALL_LEDS_OFF;
    updateHardware();
}

inline void LedDriver_TurnOn(int led_number)
{
    if (IsLedOutOfBounds(led_number))
        return;

    setLedImageBit(led_number);
    updateHardware();
}

inline void LedDriver_TurnOff(int led_number)
{
    if (IsLedOutOfBounds(led_number))
        return;

    clearLedImageBit(led_number);
    updateHardware();
}

inline void LedDriver_TurnAllOn()
{
    ledsImage = ALL_LEDS_ON;
    updateHardware();
}

inline void LedDriver_TurnAllOff()
{
    ledsImage = ALL_LEDS_OFF;
    updateHardware();
}

bool LedDriver_IsOn(int led_number)
{
    if (IsLedOutOfBounds(led_number))
        return false;

    return ledsImage & (convertLedNumberToBit(led_number));
}

bool LedDriver_IsOff(int led_number)
{
    return !LedDriver_IsOn(led_number);
}