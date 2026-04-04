#include <catch2/catch_test_macros.hpp>

#include "led_driver.hpp"

#include <cstdint>

TEST_CASE("LED driver", "[led_driver]")
{
    uint16_t virtualLeds = 0xffff;
    LedDriver_Create(&virtualLeds);

    SECTION("LEDs off after init")
    {
        REQUIRE(virtualLeds == 0);
    }

    SECTION("Turn on LED one")
    {
        LedDriver_TurnOn(1);
        REQUIRE(virtualLeds == 1);
    }

    SECTION("Turn off LED one")
    {
        LedDriver_TurnOn(1);
        LedDriver_TurnOff(1);
        REQUIRE(virtualLeds == 0);
    }

    SECTION("Turn on multiple LEDs")
    {
        LedDriver_TurnOn(9);
        LedDriver_TurnOn(8);
        REQUIRE(virtualLeds == 0x180);
    }

    SECTION("Turn All LEDs on")
    {
        LedDriver_TurnAllOn();
        REQUIRE(virtualLeds == 0xffff);
    }

    SECTION("Turn off any LED")
    {
        LedDriver_TurnAllOn();
        LedDriver_TurnOff(8);
        REQUIRE(virtualLeds == 0xff7f);
    }

    SECTION("Led memory is not readable")
    {
        virtualLeds = 0xffff;
        LedDriver_TurnOn(8);
        REQUIRE(virtualLeds == 0x80);
    }

    SECTION("Upper and lower bounds")
    {
        LedDriver_TurnOn(1);
        LedDriver_TurnOn(16);
        REQUIRE(virtualLeds == 0x8001);
    }

    SECTION("Out of bounds turns on does no harm")
    {
        LedDriver_TurnOn(-1);
        LedDriver_TurnOn(0);
        LedDriver_TurnOn(17);
        LedDriver_TurnOn(3141);
        REQUIRE(virtualLeds == 0);
    }

    SECTION("Out of bounds turns off does no harm")
    {
        LedDriver_TurnAllOn();

        LedDriver_TurnOff(-1);
        LedDriver_TurnOff(0);
        LedDriver_TurnOff(17);
        LedDriver_TurnOff(3141);
        REQUIRE(virtualLeds == 0xffff);
    }

    SECTION("Is on")
    {
        REQUIRE(!LedDriver_IsOn(11));
        LedDriver_TurnOn(11);
        REQUIRE(LedDriver_IsOn(11));
    }

    SECTION("Is off")
    {
        REQUIRE(LedDriver_IsOff(12));
        LedDriver_TurnOn(12);
        REQUIRE(!LedDriver_IsOff(12));
    }

    SECTION("Out of bounds LEDs are always off")
    {
        REQUIRE(LedDriver_IsOff(0));
        REQUIRE(LedDriver_IsOff(17));
        REQUIRE(!LedDriver_IsOn(0));
        REQUIRE(!LedDriver_IsOn(17));
    }

    SECTION("Turn off multiple LEDs")
    {
        LedDriver_TurnAllOn();
        LedDriver_TurnOff(9);
        LedDriver_TurnOff(8);
        REQUIRE(virtualLeds == ((~0x180) & 0xffff));
    }

    SECTION("All off")
    {
        LedDriver_TurnAllOn();
        LedDriver_TurnAllOff();
        REQUIRE(virtualLeds == 0);
    }
}
