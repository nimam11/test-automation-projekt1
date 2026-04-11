/**
 * @brief Unit tests for the ATmega328p timer driver.
 */
#include <cstdint>

#include <gtest/gtest.h>

#include "arch/avr/hw_platform.h"
#include "driver/timer/atmega328p.h"
#include "utils/utils.h"

#ifdef TESTSUITE

namespace driver
{
namespace
{
/** Maximum number of timer circuits available on ATmega328P. */
constexpr std::uint8_t MaxTimerCount{3U};

/** Flag to track if callback was invoked. */
bool callbackInvoked{false};

// -----------------------------------------------------------------------------
void resetCallbackFlag() noexcept { callbackInvoked = false; }

// -----------------------------------------------------------------------------
void testCallback() noexcept { callbackInvoked = true; }

// -----------------------------------------------------------------------------
constexpr std::uint32_t getMaxCount(const std::uint32_t timeout_ms) noexcept
{
    constexpr double interruptIntervalMs{0.128};
	return 0U < timeout_ms ? 
        utils::round<std::uint32_t>(timeout_ms / interruptIntervalMs) : 0U;
}

/**
 * @brief Timer initialization test.
 * 
 *        Verify that timer circuits are initialized correctly and that 
 *        resource limits are enforced.
 */
TEST(Timer_Atmega328p, Initialization)
{
    // Case 1 - Verify that only MaxTimerCount (3) timers can be used simultaneously due to 
    //          hardware limitations.
    {
        // Create MaxTimerCount timers with different timeouts.
        timer::Atmega328p timer1{100};
        timer::Atmega328p timer2{200};
        timer::Atmega328p timer3{300};

        // Verify that each timer is initialized.
        EXPECT_TRUE(timer1.isInitialized());
        EXPECT_TRUE(timer2.isInitialized());
        EXPECT_TRUE(timer3.isInitialized());

        // Create one additional timer.
        // Verify that the additional timer isn't initialized, since no circuits are available.
        timer::Atmega328p timer4{400};
        EXPECT_FALSE(timer4.isInitialized());  
    }

    // Case 2 - Verify that a timer cannot have a 0 ms timeout.
    {
        // Create a timer with a 100 ms timeout.
        // Verify that the timer is initialized.
        timer::Atmega328p validTimer{100U};
        EXPECT_TRUE(validTimer.isInitialized());

        //  Create a timer with a 0 ms timeout.
        // Verify that the timer isn't initialized (0 ms is an invalid timeout).
        timer::Atmega328p invalidTimer{0};
        EXPECT_FALSE(invalidTimer.isInitialized());
    }
}

/**
 * @brief Timer enable/disable test.
 * 
 *        Verify that timers can be started and stopped correctly.
 */
TEST(Timer_Atmega328p, EnableDisable)
{
    // Create a timer with a timeout.
    // Verify timer is not enabled initially (unless auto-started via the constructor).
    timer::Atmega328p timer{100U};
    EXPECT_FALSE(timer.isEnabled());

    // Start the timer.
    // Verify that the timer is enabled.
    timer.start();
    EXPECT_TRUE(timer.isEnabled());

    // Stop the timer.
    // Verify that the timer is disabled.
    timer.stop();
    EXPECT_FALSE(timer.isEnabled());

    // Toggle the timer.
    // Verify that the timer is enabled.
    timer.toggle();
    EXPECT_TRUE(timer.isEnabled());

    // Toggle the timer once again.
    // Verify that the timer is disabled.
    timer.toggle();
    EXPECT_FALSE(timer.isEnabled());
}

/**
 * @brief Timer timeout test.
 * 
 *        Verify that timeout values can be set and read correctly.
 */
TEST(Timer_Atmega328p, Timeout)
{
    // Create a timer with an initial timeout of 100 ms.
    // Verify timeout_ms() returns the correct value.
    timer::Atmega328p timer{100U};
    EXPECT_EQ(timer.timeout_ms(), 100U);

    // Change the timeout to 200 ms using setTimeout_ms().
    // Verify the new timeout is returned by timeout_ms().
    timer.setTimeout_ms(200);
    EXPECT_EQ(timer.timeout_ms(), 200U);

    // Change the timeout to 0 ms using setTimeout_ms().
    // Verify that the timeout is unchanged (0 ms is an invalid timeout).
    timer.setTimeout_ms(0);
    EXPECT_EQ(timer.timeout_ms(), 200U); 
}   

/**
 * @brief Timer callback test.
 * 
 *        Verify that timer callbacks are invoked when timeout occurs.
 */
TEST(Timer_Atmega328p, Callback)
{
    // Reset the callback flag (callbackInvoked) using resetCallbackFlag().
    resetCallbackFlag();
    
    // Create a timer with a short timeout, such as 10 ms, and testCallback() as callback.
    constexpr std::uint32_t timeout{10};
    timer::Atmega328p timer{timeout, testCallback};

    // Start the timer.
    timer.start();

    // Simulate timer interrupts by repeatedly calling handleCallback() on the timer.
    const auto maxCount = getMaxCount(timeout);
    for (std::uint32_t i = 0U; i < maxCount; ++i) { timer.handleCallback(); }

    // Verify that callbackInvoked is true after timeout.
    EXPECT_TRUE(callbackInvoked);
}

/**
 * @brief Timer restart test.
 * 
 *        Verify that timers can be restarted correctly.
 */
TEST(Timer_Atmega328p, Restart)
{
    // Reset the callback flag (callbackInvoked) using resetCallbackFlag().
    resetCallbackFlag();

    // Create and start a timer with testCallback() as callback.
    constexpr std::uint32_t timeout{10U};
    timer::Atmega328p timer{timeout, testCallback};
    timer.start();

    // Call handleCallback() enough times to almost reach the timeout (getMaxCount() - 1).
    const auto maxCount = getMaxCount(timeout);
    for (std::uint32_t i = 0U; i < maxCount - 1U; ++i) { timer.handleCallback(); }

     // Verify that the callback flag (callbackInvoked) is still false.
    EXPECT_FALSE(callbackInvoked);
    
    // Restart the timer.
    // Verify that the timer is still enabled after restart.
    timer.restart();
    EXPECT_TRUE(timer.isEnabled());
    
    // Call handleCallback() enough times to almost reach the timeout (getMaxCount() - 1).
    for (std::uint32_t i = 0U; i < maxCount - 1; ++i) { timer.handleCallback();}

    // Verify that the callback flag (callbackInvoked) is still false, since the timer was restarted.
    EXPECT_FALSE(callbackInvoked);
    
    // Call handleCallback() again to reach timeout.
    // Verify that the callback flag (callbackInvoked) is true due to timeout.
    timer.handleCallback();
    EXPECT_TRUE(callbackInvoked);
}
} // namespace
} // namespace driver

#endif /** TESTSUITE */
