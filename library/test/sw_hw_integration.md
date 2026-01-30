# HW/SW integration specification example

## Prerequisites
* Build and flash an ATmega328p processor.
* Run the system, open a serial terminal.

## 1. Temperature measurement
Denna sektion beskriver hur systemet mäter och visar temperaturen. Temperaturknappen gör att aktuell temperatur skrivs ut direkt, och en timer kan skriva ut temperaturen automatiskt var 60:e sekund.

### 1.1 Temperature button
* Press the temperature button.
När temperaturknappen trycks detekteras ett knapp-event.
Systemet läser temperaturen från temperatursensorn och skriver värdet till den seriella terminalen via UART.

* The temperature shall be printed in the terminal.
När temperaturknappen trycks läser systemet temperaturen från sensorn och skriver värdet till den seriella terminalen via UART.

### 1.2 Temperature timer
* Ensure that the temperature is printed every 60 seconds, 
or 60 seconds after the last pressdown.
Systemet har en temperaturtimer som är inställd på 60 sekunder. Temperaturen skrivs automatiskt ut i den seriella terminalen var 60:e sekund, eller 60 sekunder efter senaste tryck på temperaturknappen.

## 2. Toggle functionality
Denna sektion beskriver LED-blinkfunktionen som styrs av toggle-knappen. Tryck på knappen startar eller stoppar en timer som gör att LED:en blinkar med 100 ms intervall.

### 2.1 Toggle button
* Press the toggle button.
När toggle-knappen trycks första gången detekteras ett knapp-event av systemet.

* The toggle timer shall be enabled, which toggles the LED every 100 ms.
Systemet startar toggle-timern, som gör att LED:en växlar mellan på och av var 100:e millisekund. Detta får LED:en att blinka snabbt.

* Press the toggle button again.
När knappen trycks en andra gång detekteras ännu ett knapp-event av systemet.

* The toggle timer shall be disabled, which disables the LED.
Systemet stoppar toggle-timern och LED:en stängs av, så att den inte längre blinkar.


The toggle timeout is set to 100 ms in [main](../../library/source/main.cpp):

```cpp
constexpr uint32_t toggleTimerTimeout{100U};
```

