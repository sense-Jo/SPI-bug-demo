# SPI-bug-demo

### Description

Demonstrates a bug in the Zephyr SPI implementation on STM32WL. Whether other STM32 platforms are affected is still unknown.


### Reproducing the bug
![picture of hardware-setup](/doc/hardware-setup.jpg)

1. Connect a LIS12DW12 accelerometer to a NUCLEO-WL55JC STM32WL Nucleo-64 board over the Arduino SPI, using
  * Arduino Pin ` D8 (GPIO)` for LIS12DW12 `CS`
  * Arduino Pin `D11 (MOSI)` for LIS12DW12 `SDA/SDI/SDO`
  * Arduino Pin `D12 (MISO)` for LIS12DW12 `SA0/SDO`
  * Arduino Pin `D13 (SCK) ` for LIS12DW12 `SCL/SPC`

2. Compile the project by running
```
west build -p auto -b nucleo_wl55jc . -- -DSHIELD=lis2dw12
```
3. Flash it using `west flash` or `west flash --runner stm32cubeprogrammer`.
4. Power-cycle the Nucleo board


### Observed behaviour

After power-on, the NUCLEO LED2 flashes shortly, indicating that the LIS12DW12 initialization took place.

After two seconds, LED2 lights up and stays on; indicating that the application hangs in the sensor-reading-phase.

When attaching the Debugger using `west attach`, a backtrace (type `bt`) shows the following:
![screenshot of GDB](/doc/gdb-output.png)
