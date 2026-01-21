/**
 * @brief Smart temperature sensor implementation details.
 */
#include <stdint.h>

#include "driver/adc/interface.h"    // Contains the ADC interface.
#include "driver/tempsensor/smart.h" // Contains the smart sensor class.
#include "ml/lin_reg/interface.h"    // Contains the linear regression interface.
#include "utils/utils.h"             // Contains a function to round numbers.

namespace driver
{
namespace tempsensor
{
// -----------------------------------------------------------------------------
Smart::Smart(uint8_t pin, adc::Interface& adc, ml::lin_reg::Interface& linReg) noexcept
    : myAdc{adc}
    , myLinReg{linReg}
    , myPin{pin}
{
    // Enable the ADC if initialization succeeded.
    if (isInitialized()) { myAdc.setEnabled(true); }
}

// -----------------------------------------------------------------------------
bool Smart::isInitialized() const noexcept
{
    // Return true if the ADC is initialized, the temp sensor pin is a valid ADC channel,
    // and the linear regression model is trained.
    return myAdc.isInitialized() && myAdc.isChannelValid(myPin) && myLinReg.isTrained();
}

// -----------------------------------------------------------------------------
int16_t Smart::read() const noexcept
{
    // Read the temperature if the temp sensor is initialized.
    if (isInitialized())
    {
        // Calculate the input voltage with the ADC.
        const double inputVoltage{myAdc.inputVoltage(myPin)};

        // Predict the temperature based on the input voltage.
        const double predictedTemp{myLinReg.predict(inputVoltage)};

        // Return the temperature rounded to the nearest integer.
        return utils::round<int16_t>(predictedTemp);
    }
    // Return 0 if the temp sensor isn't initialized.
    return 0;
}
} // namespace tempsensor
} // namespace driver
