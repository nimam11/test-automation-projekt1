/**
 * @brief Timer driver stub.
 */
#pragma once

#include <stdint.h>

#include "driver/timer/interface.h"

namespace driver
{
namespace timer
{
class Stub final : public Interface
{
public:
    Stub(uint32_t timeout_ms = 1000U) noexcept
        : myInitialized{true}
        , myEnabled{false}
        , myTimedOut{false}
        , myTimeout_ms{timeout_ms}
    {}

    ~Stub() noexcept override = default;

    bool isInitialized() const noexcept override { return myInitialized; }

    bool isEnabled() const noexcept override { return myEnabled; }

    bool hasTimedOut() const noexcept override { return myTimedOut; }

    uint32_t timeout_ms() const noexcept override { return myTimeout_ms; }

    void setTimeout_ms(uint32_t timeout_ms) noexcept override
    {
        if (0U != timeout_ms) { myTimeout_ms = timeout_ms; }
    }

    void start() noexcept override { myEnabled = true; }

    void stop() noexcept override { myEnabled = false; }

    void toggle() noexcept override { myEnabled = !myEnabled; }

    void restart() noexcept override
    {
        myTimedOut = false;
        myEnabled = true;
    }

    void setInitialized(bool initialized) noexcept { myInitialized = initialized; }

    void setTimedOut(bool timedOut) noexcept { myTimedOut = timedOut; }

private:
    bool myInitialized;
    bool myEnabled;
    bool myTimedOut;
    uint32_t myTimeout_ms;
};
} // namespace timer
} // namespace driver
temsensor stub.h
#pragma once

#include "driver/tempsensor/interface.h"

namespace driver
{
namespace tempsensor
{
class Stub final : public Interface
{
public:
    Stub() noexcept
        : myInitialized{true}
        , myTemperature_c{0}
    {}

    ~Stub() noexcept override = default;

    bool isInitialized() const noexcept override { return myInitialized; }

    int16_t read() const noexcept override { return myTemperature_c; }

    void setInitialized(bool initialized) noexcept { myInitialized = initialized; }

    void setTemperature(int16_t temperature_c) noexcept { myTemperature_c = temperature_c; }

private:
    bool myInitialized;
    int16_t myTemperature_c;
};
} // namespace tempsensor
} // namespace driver
gpio stub.h
#pragma once

#include "driver/gpio/interface.h"

namespace driver
{
namespace gpio
{
class Stub final : public Interface
{
public:
    Stub(Direction direction = Direction::Input) noexcept
        : myInitialized{true}
        , myDirection{direction}
        , myValue{false}
        , myInterruptEnabled{true}
        , myInterruptOnPortEnabled{true}
    {}

    ~Stub() noexcept override = default;

    bool isInitialized() const noexcept override { return myInitialized; }

    Direction direction() const noexcept override { return myDirection; }

    bool read() const noexcept override { return myValue; }

    void write(bool output) noexcept override { myValue = output; }

    void toggle() noexcept override { myValue = !myValue; }

    void enableInterrupt(bool enable) noexcept override { myInterruptEnabled = enable; }

    void enableInterruptOnPort(bool enable) noexcept override
    {
        myInterruptOnPortEnabled = enable;
    }

    bool isInterruptEnabled() const noexcept { return myInterruptEnabled; }

    bool isInterruptOnPortEnabled() const noexcept { return myInterruptOnPortEnabled; }

    void setInitialized(bool initialized) noexcept { myInitialized = initialized; }

    void setDirection(Direction direction) noexcept { myDirection = direction; }

private:
    bool myInitialized;
    Direction myDirection;
    bool myValue;
    bool myInterruptEnabled;
    bool myInterruptOnPortEnabled;
};
} // namespace gpio
} // namespace driver

 