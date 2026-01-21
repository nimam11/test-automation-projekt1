/**
 * @brief Smart temperature sensor implementation.
 */
#pragma once

#include <stdint.h>

#include "driver/tempsensor/interface.h"

namespace ml
{
/** Linear regression interface. */
namespace lin_reg { class Interface; }
} // namespace ml

namespace driver
{
/** ADC (A/D converter) interface. */
namespace adc { class Interface; }

namespace tempsensor
{
/**
 * @brief Smart temperature sensor implementation.
 */
class Smart final : public Interface
{
public:
    /**
     * @brief Constructor.
     * 
     * @param[in] pin Pin the temperature sensor is connected to.
     * @param[in] adc A/D converter for reading the input voltage from the sensor.
     * @param[in] linReg Linear regression model to predict the temperature based on the 
     *                   input voltage.
     */
    explicit Smart(uint8_t pin, adc::Interface& adc, ml::lin_reg::Interface& linReg) noexcept;

    /**
     * @brief Destructor.
     */
    ~Smart() noexcept override = default;

    /**
     * @brief Check if the temperature sensor is initialized.
     * 
     * @return True if the temperature sensor is initialized, false otherwise.
     */
    bool isInitialized() const noexcept override;

    /**
     * @brief Read the temperature sensor.
     *
     * @return The temperature in degrees Celsius.
     */
    int16_t read() const noexcept override;

    Smart()                        = delete; // No default constructor.
    Smart(const Smart&)            = delete; // No copy constructor.
    Smart(Smart&&)                 = delete; // No move constructor.
    Smart& operator=(const Smart&) = delete; // No copy assignment.
    Smart& operator=(Smart&&)      = delete; // No move assignment.

private:
    /** A/D converter to read the input voltage from the sensor. */
    adc::Interface& myAdc;

    /** Linear regression model to predict the temperature based on the input voltage. */
    ml::lin_reg::Interface& myLinReg;

    /** Analog pin the temperature sensor is connected to. */
    const uint8_t myPin;
};
} // namespace tempsensor
} // namespace driver


