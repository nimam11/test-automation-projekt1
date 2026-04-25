/**
 * @brief Temperature sensor stub.
 */
#pragma once

#include <stdint.h>

#include "driver/tempsensor/smart.h"

namespace driver
{
namespace tempsensor
{
/**
 * @brief Temperature sensor stub.
 */
class Stub final : public Interface
{
public:
    /**
     * @brief Constructor.
     * 
     * @param[in] initialTemp Initial temperature (default = 0.0).
     */
    explicit Stub(const int16_t initialTemp = 0) noexcept
        : myInitialized{true}
        , myTemp{initialTemp}
    {}

    /**
     * @brief Destructor.
     */
    ~Stub() noexcept override = default;

    /**
     * @brief Check if the temperature sensor is initialized.
     * 
     * @return True if the temperature sensor is initialized, false otherwise.
     */
    bool isInitialized() const noexcept override { return myInitialized; }

    /**
     * @brief Read the temperature sensor.
     *
     * @return The temperature in degrees Celsius.
     */
    int16_t read() const noexcept override 
    { 
        // Return simulated temperature if initialized, otherwise 0.
        return myInitialized ? myTemp : 0;
    }

    /**
     * @brief Set initialization state.
     * 
     * @param[in] initialized True if initialized, false otherwise.
     */
    void setInitialized(const bool initialized) noexcept { myInitialized = initialized; }

    /**
     * @brief Set temperature.
     * 
     * @param[in] temp Simulated temperature.
     */
    void setTemperature(const int16_t temp) { myTemp = temp; }

    Stub(const Stub&)            = delete; // No copy constructor.
    Stub(Stub&&)                 = delete; // No move constructor.
    Stub& operator=(const Stub&) = delete; // No copy assignment.
    Stub& operator=(Stub&&)      = delete; // No move assignment.

private:
    /** Temperature sensor initialization state (true = initialized). */
    bool myInitialized;

    /** Simulated temperature. */
    int16_t myTemp;
};
} // namespace tempsensor
} // namespace driver
