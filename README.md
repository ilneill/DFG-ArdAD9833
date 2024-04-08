# DFG-ArdAD9833
A Digital Frequency Generator using an Arduino Nano and an AD9833 Module.

This is a quick and easy project to build a 0-1MHz digital frequency generator!

Features
- Uses a DollaTek AD9833 (+ MCP41010) module (SPI bus).
- Sine and Square wave generation.
- Frequency range 0 through 1MHz.
  * This is software bounded - for sine waves the AD9833 can reach 10MHz.
- Full control via a KY-040 rotary encoder.
- At a glance settings on a 20 column x 4 line LCD display (I2C bus).
- Uses open source Arduino libraries for all the hardware modules.

_Blue trace: Raw AD9833 output / Red trace: Amplified output_

![](MyArdAD9833Project.jpg) ![](WaveGen1kSquare.jpg)

## Rotary Encoder Controls
- Turn the knob to inc/dec either the frequency or the gain.
- Button long press to toggle frequency or gain control mode.
- Button double-click to toggle sine and square wave output.
- Button single click to change the freqency inc/dec step (freq mode).
- Button single click to change the gain low/med/hi setting (gain mode).

## Arduino Sketches
### P1 Sketch
**_FreqGenADMCP-P1.ino_**

Halfway stage: Just the Rotary Encoder and the LCD modules are active.
### P2 Sketch
**_FreqGenADMCP-P2.ino_**

Complete: The DollaTek AD9833 and MCP41010 module is also active.

## Hardware
- Arduino Nano (my go-to Arduino board for quick projects).
- DollaTek AD9833 frequency generator module with onboard MCP41010 and op-amp.
  * https://www.amazon.co.uk/dp/B07DJT8Z49?psc=1&ref=ppx_yo2ov_dt_b_product_details
    
  ![](DollaTekAD9833Module500.jpg)
- KY-040 rotary encoder module.
  
  ![](KY-040Module500.jpg)
- HD44780 based 20x4 LCD display.
  
  ![](LCD2004Module500.jpg)
- HW-061 I2C to HD44780 piggyback module.
- Owon VDS1022i USB oscillosope with open source software.
  * https://github.com/florentbr/OWON-VDS1022 

## Circuit Design
**_WaveGenAD9833v3.fzz_**

![](WaveGenAD9833v3.png)

## Example Waveforms
### Sine Wave (500kHz)
_Blue trace: Raw AD9833 output / Red trace: Amplified output_

![](WaveGen500kSine.png)
### Square Wave (500kHz)
_Blue trace: Raw AD9833 output / Red trace: Amplified output_

![](WaveGen500kSquare.png)

## YouTube...
My project build, code explanation and demonstration on an oscilloscope here:
 - https://youtu.be/ComingSoon...

## **_Enjoy!_**
