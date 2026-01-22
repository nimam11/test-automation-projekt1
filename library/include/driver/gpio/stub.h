/**
 * @brief GPIO driver stub.
 */
#pragma once

#include <stdint.h>

#include "driver/gpio/interface.h"

namespace driver
{
namespace gpio
{
/**
 * @brief GPIO driver stub.
 */
class Stub final : public Interface
{
public:
    /**
     * @brief Constructor.
     */
    Stub() noexcept
        : myInitialized{true}
        , myValue{true}
        , myInterruptEnabled{false}
    {}

    /** 
     * @brief Destructor.
     */
    ~Stub() noexcept override = default;

    /**
     * @brief Check whether the GPIO is initialized.
     * 
     *        An uninitialized device indicates that the specified PIN was unavailable or invalid
     *        when the device was created.
     * 
     * @return True if the device is initialized, false otherwise.
     */
    bool isInitialized() const noexcept override { return myInitialized; }

    /**
     * @brief Get the data direction of the GPIO.
     * 
     * @return The data direction of the GPIO.
     */
    Direction direction() const noexcept override
    {
        // Data direction is irrelevant for the stub, always return input.
        return Direction::Input;
    }

    /**
     * @brief Read input of the GPIO.
     * 
     * @return True if the input is high, false otherwise.
     */
    bool read() const noexcept override { return myValue; }
    /**
     * @brief Write output to the GPIO.
     * 
     * @param[in] output The output value to write (true = high, false = low).
     */
    void write(bool output) noexcept override
    {
        // Only set the GPIO value if the device is initialized, otherwise do nothing.
        if (myInitialized) { myValue = output; }
    }

    /**
     * @brief Toggle the output of the GPIO.
     */
    void toggle() noexcept override
    {
        // Only toggle the GPIO value if the device is initialized, otherwise do nothing.
        if (myInitialized) { myValue = !myValue;}
    }

    /**
     * @brief Enable/disable pin change interrupt for the GPIO.
     * 
     * @param[in] enable True to enable pin change interrupt for the GPIO, false otherwise.
     */
    void enableInterrupt(bool enable) noexcept override
    {
        // Only update the interrupt state if the device is initialized, otherwise do nothing.
        if (myInitialized) { myInterruptEnabled = enable; }
    }

    /**
     * @brief Enable pin change interrupt for I/O port associated with the GPIO.
     * 
     * @param[in] enable True to enable pin change interrupt for the I/O port, false otherwise.
     */
    void enableInterruptOnPort(bool enable) noexcept override
    {
        // Only update the interrupt state if the device is initialized, otherwise do nothing.
        if (myInitialized) { myInterruptEnabled = enable; }
    }

    /**
     * @brief Check whether interrupts are enabled for the GPIO.
     * 
     * @return True if interrupts are enabled, false otherwise.
     */
    bool isInterruptEnabled() const noexcept { return myInterruptEnabled; }

    /**
     * @brief Set initialization state for testing.
     * 
     *        If the device is set to uninitialized, the GPIO value is set to low and
     *        interrupts are disabled.
     * 
     * @param[in] initialized True if the device is initialized, false otherwise.
     */
    void setInitialized(bool initialized) noexcept
    {
        // Store the initialization state.
        myInitialized = initialized;

        // Set the GPIO value to low and disable interrupts if the device is uninitialized.
        if (!myInitialized)
        {
            myValue            = false;
            myInterruptEnabled = false;
        }
    }

private:
    /** GPIO initialization status (true = initialized). */
    bool myInitialized;

    /** GPIO value (true = high, false = low). */
    bool myValue;

    /** GPIO interrupt status (true = enabled, false = disabled). */
    bool myInterruptEnabled;
};
} // namespace gpio
} // namespace driver
