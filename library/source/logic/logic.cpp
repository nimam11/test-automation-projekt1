/**
 * @brief Generic logic implementation details for an MCU with configurable hardware devices.
 */
#include <stdint.h>

#include "driver/adc/interface.h"
#include "driver/eeprom/interface.h"
#include "driver/gpio/interface.h"
#include "driver/serial/interface.h"
#include "driver/tempsensor/interface.h"
#include "driver/timer/interface.h"
#include "driver/watchdog/interface.h"
#include "logic/logic.h"

namespace logic
{
// -----------------------------------------------------------------------------
Logic::Logic(driver::gpio::Interface& led,
             driver::gpio::Interface& toggleButton,
             driver::gpio::Interface& tempButton, 
             driver::timer::Interface& debounceTimer, 
             driver::timer::Interface& toggleTimer,
             driver::timer::Interface& tempTimer,
             driver::serial::Interface& serial, 
             driver::watchdog::Interface& watchdog, 
             driver::eeprom::Interface& eeprom, 
             driver::tempsensor::Interface& tempSensor) noexcept
    : myLed{led}
    , myToggleButton{toggleButton}
    , myTempButton{tempButton}
    , myDebounceTimer{debounceTimer}
    , myToggleTimer{toggleTimer}
    , myTempTimer{tempTimer}
    , mySerial{serial}
    , myWatchdog{watchdog}
    , myEeprom{eeprom}
    , myTempSensor{tempSensor}
{
    // Enable system if all hardware drivers were initialized correctly.
    if (isInitialized())
    {
        myToggleButton.enableInterrupt(true);
        myTempButton.enableInterrupt(true);
        myTempTimer.start();
        mySerial.setEnabled(true);
        myWatchdog.setEnabled(true);
        myEeprom.setEnabled(true);

        // Enable the toggle timer if it was enabled before poweroff.
        restoreToggleStateFromEeprom();
    }
}

// -----------------------------------------------------------------------------
Logic::~Logic() noexcept
{
    // Disable system.
    myLed.write(false);
    myToggleButton.enableInterrupt(false);
    myTempButton.enableInterrupt(false);
    myDebounceTimer.stop();
    myToggleTimer.stop();
    myTempTimer.stop();
    mySerial.setEnabled(false);
    myWatchdog.setEnabled(false);
    myEeprom.setEnabled(false);
    myToggleTimer.stop();
}

// -----------------------------------------------------------------------------
bool Logic::isInitialized() const noexcept
{
    // Return true if all hardware drivers are initialized.
    return myLed.isInitialized() && myToggleButton.isInitialized() && myTempButton.isInitialized()
        && myDebounceTimer.isInitialized() && myToggleTimer.isInitialized() 
        && myTempTimer.isInitialized() && mySerial.isInitialized() && myWatchdog.isInitialized()
        && myEeprom.isInitialized() && myTempSensor.isInitialized();
}

// -----------------------------------------------------------------------------
void Logic::run(const bool& stop) noexcept
{
    // Terminate the function if initialization failed.
    if (!isInitialized())
    {
        // Print an error message is the serial device driver is working.
        if (mySerial.isInitialized()) 
        { 
            const bool enabled{mySerial.isEnabled()};
            mySerial.setEnabled(true);
            mySerial.printf("Failed to run the system: initialization failed!\n");
            mySerial.setEnabled(enabled);
        }
        return;
    }

    // Run the system continuously.
    mySerial.printf("Running the system!\n");

    // Print info about transmitting commands.
    mySerial.printf("Please enter one of the following commands:\n");
    mySerial.printf("- 't' to toggle the toggle timer\n");
    mySerial.printf("- 'r' to read the temperature\n");
    mySerial.printf("- 's' to check the state of the toggle timer\n\n");

    while (!stop) 
    { 
        // Regularly reset the watchdog to avoid system reset.
        myWatchdog.reset();

        // Read serial port, execute received commands.
        readSerialPort();
    }
}

// -----------------------------------------------------------------------------
void Logic::handleButtonEvent() noexcept
{
    // Ignore if this call was done manually.
    if (myDebounceTimer.isEnabled()) { return; }
    
    // Disable interrupts on the I/O ports to mitigate effects of debouncing.
    myToggleButton.enableInterruptOnPort(false);
    myTempButton.enableInterruptOnPort(false);
    myDebounceTimer.start();

    // Handle specific button event when pressed.
    if (myToggleButton.read()) { handleToggleButtonPressed(); }
    if (myTempButton.read()) { handleTempButtonPressed(); }
}

// -----------------------------------------------------------------------------
void Logic::handleDebounceTimerTimeout() noexcept
{
    // Re-enable interrupts on the ports after debounce timer timeout.
    if (myDebounceTimer.hasTimedOut())
    {
        myDebounceTimer.stop();
        myToggleButton.enableInterruptOnPort(true);
        myTempButton.enableInterruptOnPort(true);
    }
}

// -----------------------------------------------------------------------------
void Logic::handleToggleTimerTimeout() noexcept 
{
    // Toggle the LED on toggle timer timeout. 
    if (myToggleTimer.hasTimedOut()) { myLed.toggle(); }
}

// -----------------------------------------------------------------------------
void Logic::handleTempTimerTimeout() noexcept 
{ 
    // Read and print the temperature on temperature timer timeout.
    if (myTempTimer.hasTimedOut()) { printTemperature(); }
}

// -----------------------------------------------------------------------------
void Logic::writeToggleStateToEeprom(const bool enable) noexcept
{ 
    myEeprom.write(ToggleStateAddr, static_cast<uint8_t>(enable));
}

// -----------------------------------------------------------------------------
bool Logic::readToggleStateFromEeprom() const noexcept
{
    uint8_t state{};
    return myEeprom.read(ToggleStateAddr, state) ? static_cast<bool>(state) : false;
}

// -----------------------------------------------------------------------------
void Logic::printTemperature() noexcept
{
    // Read and print the temperature.
    const int16_t temperature{myTempSensor.read()};
    mySerial.printf("Temperature: %d Celsius\n", temperature);
}

// -----------------------------------------------------------------------------
void Logic::handleToggleButtonPressed() noexcept
{
    // Toggle the toggle timer on pressdown, safe the current LED state in EEPROM.
    myToggleTimer.toggle();
    writeToggleStateToEeprom(myToggleTimer.isEnabled());

    if (myToggleTimer.isEnabled()) { mySerial.printf("Toggle timer enabled!\n"); }
    else
    {
        // Immediately disable the LED if the toggle timer is disabled to ensure that the LED
        // isn't stuck in an enabled state.
        mySerial.printf("Toggle timer disabled!\n");
        myLed.write(false);
    }
}

// -----------------------------------------------------------------------------
void Logic::handleTempButtonPressed() noexcept
{
    // Read and print the temperature on pressdown.
    // Restart the temperature timer.
    printTemperature();
    myTempTimer.restart();
}

// -----------------------------------------------------------------------------
void Logic::restoreToggleStateFromEeprom() noexcept
{
    // Start the toggle timer if the LED was enabled before poweroff.
    if (readToggleStateFromEeprom())
    {
        myToggleTimer.start();
        mySerial.printf("Toggle timer enabled!\n");
    }
}

// -----------------------------------------------------------------------------
bool Logic::readSerialPort() noexcept
{
    // Buffer size (bytes).
    constexpr uint16_t bufferSize{5U};

    // Read timeout in milliseconds.
    constexpr uint16_t readTimeout_ms{100U};

    // Read buffer (to receive data as bytes).
    uint8_t buffer[bufferSize]{};

    // Read the serial port, store received data in the buffer.
    const int16_t bytesRead{mySerial.read(buffer, bufferSize, readTimeout_ms)};

    // Check the return value, return false if the operation failed.
    if (0 > bytesRead)
    {
        mySerial.printf("Failed to receive data from the serial port!\n");
        return false;
    }

    // Handle command if we received data.
    if (0 < bytesRead)
    {
        // Extract the transmitted command, if any.
        const char cmd{static_cast<char>(buffer[0U])};

        // Handle received command.
        switch (cmd)
        {
            // If we received command 't', toggle the toggle timer.
            case 't':
            {
                // Command 't' works the same as pressing the toggle button.
                handleToggleButtonPressed();
                break;
            }
            // If we received command 'r', read the temperature.
            case 'r':
            {
                // Command 'r' works the same as pressing the temperature button.
                handleTempButtonPressed();
                break;
            }
            // If we received command 's', print the state of the toggle timer.
            case 's':
            {
                const char* state{myToggleTimer.isEnabled() ? "enabled" : "disabled"};
                mySerial.printf("The toggle timer is %s!\n", state);
                break;
            }
            // Print error message if an unknown command command was entered.
            default:
            {
                mySerial.printf("Unknown command %c!\n", cmd);
                return false;
            }
        }
    }
    // Return true to indicate success.
    return true;
}
} // namespace logic