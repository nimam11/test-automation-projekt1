/**
 * @brief Unit tests for the smart temperature sensor.
 */
#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "driver/adc/stub.h"
#include "driver/tempsensor/smart.h"
#include "ml/lin_reg/fixed.h"
#include "ml/types.h"
#include "utils/utils.h"

#ifdef TESTSUITE

namespace driver
{
namespace
{
// -----------------------------------------------------------------------------
constexpr double computeInputVoltage(const std::uint16_t adcVal) noexcept
{
    constexpr double supplyVoltage{5.0};
    constexpr std::uint16_t adcMax{1023U};

    // Convert the ADC value to a voltage.
    return static_cast<double>(adcVal) / adcMax * supplyVoltage;
}

// -----------------------------------------------------------------------------
constexpr std::int16_t convertToTemp(const double inputVoltage) noexcept
{
    // Convert voltage to temperature: T(°C) = 100 * V - 50.
    return utils::round<std::int16_t>(100.0 * inputVoltage - 50.0); 
}

// -----------------------------------------------------------------------------
constexpr std::int16_t convertToTemp(const std::uint16_t adcVal) noexcept
{
    // Convert ADC value to voltage, then to temperature.
    return convertToTemp(computeInputVoltage(adcVal));
}

// -----------------------------------------------------------------------------
bool trainModel(ml::lin_reg::Fixed& model, const std::size_t epochCount = 1000U,
                const double learningRate = 0.01) noexcept
{
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

/**
 * @brief Smart temp sensor initialization test.
 * 
 *        Verify that the sensor isn't initialized if:
 *            - The ADC isn't initialized.
 *            - The temp sensor pin number (the ADC channel) is invalid.
 *            - The linear regression model is untrained.
 */
TEST(TempSensor_Smart, Initialization)
{
    constexpr std::uint16_t adcVal{100U};
    constexpr std::int16_t defaultTemp{0U};
    constexpr std::int16_t expectedTemp{convertToTemp(adcVal)};

    // Set up the ADC.
    adc::Stub adc{};
    adc.setValue(adcVal);

    // Set up the linear regression model.
    // Train the model to predict the temperature based on the input voltage, expect success.
    ml::lin_reg::Fixed linReg{};
    EXPECT_TRUE(trainModel(linReg));
    EXPECT_TRUE(linReg.isTrained());

     // Case 1 - Simulate a valid pin.
    {
        // Create a temp sensor instance for this pin.
        constexpr std::uint8_t pin{0U};
       
        // Set ADC channel validity to true (simulate a valid pin).
        adc.setChannelValidity(true);

        //! Simulate that the ADC is initialized.
        adc.setInitialized(true);

        // Create a smart temp sensor.
        tempsensor::Smart tempSensor{pin, adc, linReg};
        
        // For valid pins, the sensor should be initialized and return the expected temperature.
        EXPECT_TRUE(tempSensor.isInitialized());
        EXPECT_EQ(tempSensor.read(), expectedTemp);
    }

    // Case 2 - Simulate an invalid pin.
    {
        // Create a temp sensor instance for this pin.
        constexpr std::uint8_t pin{10U};

        // Set ADC channel validity to false (simulate an invalid pin).
        adc.setChannelValidity(false);

        //! Simulate that the ADC is initialized.
        adc.setInitialized(true);

        // Create a smart temp sensor.
        tempsensor::Smart tempSensor{pin, adc, linReg};

        // Expect the temp sensor to not be initialized and to return the default temperature.
        EXPECT_FALSE(tempSensor.isInitialized());
        EXPECT_EQ(tempSensor.read(), defaultTemp);
    }

    // Case 3 - Simulate that the ADC isn't initialized.
    {
        // Create a temp sensor instance.
        constexpr std::uint8_t pin{0U};

        // Set ADC channel validity to true (simulate a valid pin).
        adc.setChannelValidity(true);

        // Simulate that the ADC isn't initialized.
        adc.setInitialized(false);

        // Create a smart temp sensor here.
        tempsensor::Smart tempSensor{pin, adc, linReg};
        
        // Expect the temp sensor to not be initialized and to return the default temperature.
        EXPECT_FALSE(tempSensor.isInitialized());
        EXPECT_EQ(tempSensor.read(), defaultTemp);
    }

    // Case 4 - Simulate that the linear regression model isn't trained.
    {
        // Create a temp sensor instance.
        constexpr std::uint8_t pin{0U};

        // Set up an untrained linear regression model.
        ml::lin_reg::Fixed untrainedModel{};
        EXPECT_FALSE(untrainedModel.isTrained());

        // Set ADC channel validity to true (simulate a valid pin).
        adc.setChannelValidity(true);

        // Simulate that the ADC is initialized.
        adc.setInitialized(true);

        // Create a smart temp sensor here, use the untrained model.
        tempsensor::Smart tempSensor{pin, adc, untrainedModel};

        // Expect the temp sensor to not be initialized and to return the default temperature.
        EXPECT_FALSE(tempSensor.isInitialized());
        EXPECT_EQ(tempSensor.read(), defaultTemp);
    }
}

/**
 * @brief Smart temp sensor happy path test.
 * 
 *        Verify that the temp sensor predicts accurately when the model is properly trained 
 *        and valid input is provided.
 */
TEST(TempSensor_Smart, HappyPath)
{
    constexpr std::uint8_t tempSensorPin{0U};
    constexpr std::uint16_t adcMax{1000U};
    constexpr std::size_t stepVal{10U};

    // Set up the ADC.
    adc::Stub adc{};
    adc.setInitialized(true);
    adc.setChannelValidity(true);

    // Set up and train the linear regression model.
    ml::lin_reg::Fixed linReg{};
    EXPECT_TRUE(trainModel(linReg));
    
    // Set up the temp sensor, use the interface this time.
    std::unique_ptr<tempsensor::Interface> tempSensor{
        std::make_unique<tempsensor::Smart>(tempSensorPin, adc, linReg)};

    // Expect the temp sensor to be initialized successfully.
    EXPECT_TRUE(tempSensor->isInitialized());

    // Try different ADC values to simulate different input voltages.
    for (std::uint16_t adcVal{}; adcVal <= adcMax; adcVal += stepVal)
    {
        //! Calculate the expected temperature for this ADC value.
        const std::int16_t expectedTemp{convertToTemp(adcVal)};

        // Set the ADC register to simulate the sensor reading.
        adc.setValue(adcVal);

        // The sensor should return the expected temperature for this ADC value.
        EXPECT_EQ(tempSensor->read(), expectedTemp);
    }
}
} // namespace
} // namespace driver.

#endif /** TESTSUITE */
