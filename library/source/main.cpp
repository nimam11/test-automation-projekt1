/**
 * @brief Demonstration of GPIO device drivers in C++.
 * 
 *        The following devices are used:
 *            - A button to toggle a blink timer.
 *            - A button to read the surrounding temperature.
 *            - A blink timer to toggle an LED when enabled.
 *            - A temperature timer to print the temperature on timeout.
 *            - A debounce timer to reduce the effect of contact bounces after pushing the buttons.
 *            - A serial device to print serial data via UART.
 *            - A watchdog timer to restart the program if it gets stuck somewhere.
 *            - An EEPROM stream to store the LED state. On startup, this value is read; if the
 *              last stored state before power down was "on," the LED will automatically blink.
 *            - A temperature sensor to read the surrounding temperature.
 */
#include "driver/adc/atmega328p.h"
#include "driver/eeprom/atmega328p.h"
#include "driver/gpio/atmega328p.h"
#include "driver/serial/atmega328p.h"
#include "driver/tempsensor/smart.h"
#include "driver/timer/atmega328p.h"
#include "driver/watchdog/atmega328p.h"
#include "logic/logic.h"
#include "ml/lin_reg/fixed.h"
#include "ml/types.h"

using namespace driver;

namespace
{
/** Pointer to the logic implementation. */
logic::Interface* myLogic{nullptr};

namespace callback
{
/**
 * @brief Callback for the buttons.
 * 
 *        This callback is invoked when a button event occurs.
 */
void button() noexcept { myLogic->handleButtonEvent(); }

/**
 * @brief Callback for the debounce timer.
 * 
 *        This callback is invoked when the debounce timer times out.
 */
void debounceTimer() noexcept { myLogic->handleDebounceTimerTimeout(); }

/**
 * @brief Callback for the toggle timer.
 * 
 *        This callback is invoked when the toggle timer times out.
 */
void toggleTimer() noexcept { myLogic->handleToggleTimerTimeout(); }

/**
 * @brief Callback for the temperature timer.
 * 
 *        This callback is invoked when the temperature timer times out.
 */
void tempTimer() noexcept { myLogic->handleTempTimerTimeout(); }

} // namespace callback

/**
 * @brief Train fixed linear regression model to predict temperature based on the input voltage.
 * 
 * @param[in] model The model to train.
 * 
 * @return True on success, false on failure.
 */
bool trainModel(ml::lin_reg::Fixed& model) noexcept
{
    // Training parameters.
    constexpr size_t epochCount{100U};
    constexpr double learningRate{0.01};

    // Training data to teach the model to predict T = 100 * Uin - 50.
    const ml::Matrix1d trainIn{0.0, 0.1, 0.2, 0.3, 0.4, 
                               0.5, 0.6, 0.7, 0.8, 0.9, 
                               1.0, 1.1, 1.2, 1.3, 1.4};
    const ml::Matrix2d trainOut{-50.0, -40.0, -30.0, -20.0, -10.0, 
                                0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 
                                60.0, 70.0, 80.0, 90.0, 100.0};

    // Train the model, return the result.
    return model.train(trainIn, trainOut, epochCount, learningRate);
}
} // namespace

/**
 * @brief Initialize and run the system on the target MCU.
 * 
 * @return 0 on termination of the program (should never occur).
 */
int main()
{
    // Set pin numbers.
    constexpr uint8_t tempSensorPin{2U};
    constexpr uint8_t ledPin{8U};
    constexpr uint8_t toggleButtonPin{12U};
    constexpr uint8_t tempButtonPin{13U};

    // Set timeouts.
    constexpr uint32_t debounceTimerTimeout{300U};
    constexpr uint32_t toggleTimerTimeout{100U};
    constexpr uint32_t tempTimerTimeout{60000U};

    constexpr auto input{gpio::Direction::InputPullup};
    constexpr auto output{gpio::Direction::Output};

    // Initialize the GPIO devices.
    gpio::Atmega328p led{ledPin, output};
    gpio::Atmega328p toggleButton{toggleButtonPin, input, callback::button};
    gpio::Atmega328p tempButton{tempButtonPin, input, callback::button};

    // Initialize the timers.
    timer::Atmega328p debounceTimer{debounceTimerTimeout, callback::debounceTimer};
    timer::Atmega328p toggleTimer{toggleTimerTimeout, callback::toggleTimer};
    timer::Atmega328p tempTimer{tempTimerTimeout, callback::tempTimer};

    // Obtain a reference to the singleton serial device instance.
    auto& serial{serial::Atmega328p::getInstance()};
    serial.setEnabled(true);

    // Obtain a reference to the singleton watchdog timer instance.
    auto& watchdog{watchdog::Atmega328p::getInstance()};

    // Obtain a reference to the singleton EEPROM instance.
    auto& eeprom{eeprom::Atmega328p::getInstance()};

    // Obtain a reference to the singleton ADC instance.
    auto& adc{adc::Atmega328p::getInstance()};

    // Create a linear regression model that predicts temperature based on input voltage.
    ml::lin_reg::Fixed linReg{};

    // Train the model and print the result.
    if (trainModel(linReg))
    {
        serial.printf("Temperature prediction training succeeded!\n");
    }
    else { serial.printf("Temperature prediction training failed!\n"; )}

    // Initialize the smart temperature sensor.
    tempsensor::Smart tempSensor{tempSensorPin, adc, linReg};

    // Initialize the logic implementation with the given hardware.
    logic::Logic logic{led, 
                       toggleButton, 
                       tempButton, 
                       debounceTimer, 
                       toggleTimer, 
                       tempTimer,
                       serial, 
                       watchdog, 
                       eeprom, 
                       tempSensor};
    myLogic = &logic;

    // Run the application on the target MCU.
    const bool stop{false};
    myLogic->run(stop);
    return 0;
}
