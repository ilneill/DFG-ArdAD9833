/*!
 * Digital Waveform Generator using an AD9833 and MCP41010 Digital Potentiometer.
 *  Designed for, and tested on, an Arduino Uno R3 and an Arduino Nano R3.
 *  Does not compile for the Arduino Uno R4 - issues with at least one of the libraries.
 * 
 *  Based upon the Dollatek AD9833 (+MCP41010) module:
 *   https://www.amazon.co.uk/dp/B07DJT8Z49?psc=1&ref=ppx_yo2ov_dt_b_product_details
 *  Using a KY-040 Rotary Encoder module.
 *  Using a 2004 LCD discply with HD44780 controller and HW-061 I2C piggyback board
 * 
 * (c) 2024 Ian Neill, arduino@binaria.co.uk
 *  Licence: GPLv3
 * 
 * References:
 *  https://circuitdigest.com/microcontroller-projects/build-your-own-function-generator-with-arduino-and-ad9833-module
 *  https://www.best-microcontroller-projects.com/mcp41010.html
 *  https://github.com/sleemanj/MCP41_Simple/tree/master
 *  https://www.pjrc.com/teensy/td_libs_Encoder.html
 *  https://arduinogetstarted.com/tutorials/arduino-lcd-i2c
 */

// ********************
// * Main Sketch File *
// ********************

#include <AD9833.h>
#include <MCP41_Simple.h>
#include <EncoderButton.h>
#include <LiquidCrystal_I2C.h>

// Define all the I/O pin names.
#define SPICLKPIN   13      // Hardware SPI.
#define SPIDINPIN   12      // Hardware SPI.
#define SPIDOUTPIN  11      // Hardware SPI.
#define AD9833PIN   10      // AD9833 select pin.
#define MCP41010PIN 9       // MCP41010 select pin.
#define SPAREPIND8  8
#define SPAREPIND7  7
#define SPAREPIND6  6
#define SPAREPIND5  5
#define ENCODERSWI  4       // Rotary Encoder button pin.
#define ENCODERCLK  3       // Rotary Encoder clock pin.
#define ENCODERDAT  2       // Rotary Encoder data pin.
#define SPAREPIND1  1
#define SPAREPIND0  0
#define SPAREPINA7  A7      // Not available on the Arduino Uno!
#define SPAREPINA6  A6
#define I2CSCLPIN   A5      // Hardware I2C.
#define I2CSDAPIN   A4      // Hardware I2C.
#define SPAREPINA3  A3
#define SPAREPINA2  A2
#define SPAREPINA1  A1
#define SPAREPINA0  A0
#define LCDADDR     0x27    // LCD I2C address.
#define BAUDRATE    115200  // Serial speed.

// Define the frequency limits for the AD9833.
#define FREQMIN     0       // Min frequency for a sine or square wave.
#define FREQMAXSIN  1.25E7  // Max frequency for a sine wave is 12.5MHz.
#define FREQMAXSQR  1.00E6  // Max frequency for a square wave is 1.0MHz.
#define FREQMAXTRI  1.00E6  // Max frequency for a triangle wave is 1.0MHz.
#define FREQSTEPMIN 1       // Min frequency step for a sine or square wave.
#define FREQSTEPMAX 100000  // Max frequency step for a sine or square wave.

// Instantiate an AD9833 object.
AD9833            myAD9833(AD9833PIN);    // Hardware SPI and select pin AD9833PIN.
// Instantiate an MCP41010 object.
MCP41_Simple      myMCP41010;             // Digital potentiometer - 10K.
// Instantiate an EncoderButton object.
EncoderButton     myRotEnc(ENCODERDAT, ENCODERCLK, ENCODERSWI); // A rotary encoder with a button using hardware interrupt pins.
// Instantiate an LCD object - 20 x 4 display.
LiquidCrystal_I2C myLCD(LCDADDR, 20, 4);  // I2C address LCDADDR, 20 columns X 4 rows.

// Initialise some global variables with starting values.
byte    waveType  = AD9833_SQUARE1;       // Start with a square wave.
int32_t frequency = 1000;                 // Start with a frequency of 1.0kHz
int32_t freqStep  = 100;                  // Start with 100Hz frequency step.
byte    rdValue   = 128;                  // MCP41010 resistor divider value.
bool    encChange = true;                 // Has one of the encoder handlers detected a change?
bool    freqMode  = true;                 // Does turning the encoder change the frequency (T) or the gain (F)?

void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("AD9833 Frequency Generator");
  Serial.println();
  // Initialise the hardware SPI interface.
  SPI.begin();  
  // Initialise the AD9833.
  myAD9833.begin();
  myAD9833.setWave(waveType);                         // Set the initial wave type.
  myAD9833.setFrequency(frequency, 0);                // Set the initial frequency on channel 0.
  // Initialise the MCP41010.
  myMCP41010.begin(MCP41010PIN);                      // Hardware SPI and select pin MCP41010PIN.
  myMCP41010.setWiper(rdValue);                       // Set the initial potentiometer wiper value.
  //pinMode(MCP41010PIN, OUTPUT);
  //writeMCP41010(MCP41010PIN, rdValue);
  // Initialise the Rotary Encoder.
  myRotEnc.setClickHandler(onRotEncClicked);          // Button single and multi-click handler.
  myRotEnc.setLongPressHandler(onRotEncLPress, true); // Button long-click handler.
  myRotEnc.setEncoderHandler(onRotEncEncoder);        // Encoder handler.
  // Initialise the LCD.
  myLCD.init();
  myLCD.backlight();
}

void loop() {
  // Has anything been flagged as changed by the rotary encoder event handlers?
  if (encChange) {
    // There has been a change so update things...
    encChange = false;            // Clear the update flag.
    // Update the AD9833 with the new wave type and frequency..
    myAD9833.setWave(waveType);
    myAD9833.setFrequency(frequency, 0);
    // Update the MCP41010 with the new gain.
    myMCP41010.setWiper(rdValue);
    //writeMCP41010(MCP41010PIN, rdValue);
    // Update the LCD with the changes.
    myLCD.clear();                // Clear the display.
    myLCD.setCursor(0, 0);        // Move cursor to column 0, row 0.
    myLCD.print("Wave: ");
    if (myAD9833.getWave() == AD9833_SINE) {
      myLCD.print("Sine");
    }
    else if (myAD9833.getWave() == AD9833_SQUARE1) {
      myLCD.print("Square");
    }
    else if (myAD9833.getWave() == AD9833_TRIANGLE) {
       myLCD.print("Triangle");
    }
    else {
      myLCD.print("???");
    }
    myLCD.setCursor(0, 1);        // Move cursor to column 0, row 1.
    myLCD.print("Freq: ");
    myLCD.print(int32_t(myAD9833.getFrequency()));
    myLCD.print("Hz");
    myLCD.setCursor(2, 2);        // Move cursor to column 2, row 2.
    myLCD.print("Step: ");
    myLCD.print(freqStep);
    myLCD.print("Hz");
    myLCD.setCursor(0, 3);        // Move cursor to column 0, row 3.
    myLCD.print("Gain: ");
    myLCD.print(rdValue);
   if (freqMode) {
      myLCD.setCursor(18, 1);     // Move cursor to column 18, row 1.
    }
    else {
      myLCD.setCursor(18, 3);     // Move cursor to column 18, row 3.
    }
    myLCD.print("<=");
    // Update the serial with the changes.
    Serial.print("Wave Type: ");
    Serial.println(myAD9833.getWave());
    Serial.print("Frequency: ");
    Serial.print(int32_t(myAD9833.getFrequency()));
    Serial.println("Hz");
    Serial.print("  Step: ");
    Serial.print(freqStep);
    Serial.println("Hz");
    Serial.print("Gain: ");
    Serial.println(rdValue);
  }
  // Update the Rotary Encoder.
  myRotEnc.update();
}

/* // Set the Resistance Divider Value in the MCP41010.
void writeMCP41010(byte mcpSelPin, byte rdValue) {
  digitalWrite(mcpSelPin,LOW);    // Take the select pin low - select.
  SPI.transfer(B00010001);        // Send the command byte - always 0x11 for the MCP41010.
  SPI.transfer(rdValue);          // Send the resistance divider value.
  digitalWrite(mcpSelPin, HIGH);  // Take the select pin high - de-select.
  SPI.endTransaction();
} */

// Handler for the Rotary Encoder 'clicked' event.
void onRotEncClicked(EncoderButton& eb) {
  Serial.print("Encoder clickCount: ");
  Serial.println(eb.clickCount());
  // A single click controls the frequency or the gain.
  if (eb.clickCount() == 1) {
    // Frequency Mode = true - clicks change the frequency step: 1 10 100 1000 10000 100000.
    if (freqMode) {
      freqStep *= 10;
      if (freqStep > FREQSTEPMAX) {
        freqStep = FREQSTEPMIN;
      }
    }
    // Frequency Mode = false - clicks change the gain: 0 128 255.
    else {
      if (rdValue == 0) {
        rdValue = 128;
      }
      else if (rdValue == 128) {
        rdValue = 255;
      }
      else if (rdValue == 255) {
        rdValue = 0;
      }
      else {
        rdValue = 128;
      }
    }
  }
  // Not a single click - change the wave type.
  else {
    if (waveType == AD9833_SINE) {
      waveType = AD9833_SQUARE1;
      // Check that the frequency is not too high for a square wave.
      if (frequency > FREQMAXSQR) {
        frequency = FREQMAXSQR;
      }
    }
    else if (waveType == AD9833_SQUARE1) {
      waveType = AD9833_TRIANGLE;
      // Check that the frequency is not too high for a triangle wave.
      if (frequency > FREQMAXTRI) {
        frequency = FREQMAXTRI;
      }
    }
    else {
      waveType = AD9833_SINE;
    }
  }
  encChange = true;
}
// Handler for the Rotary Encoder 'long press' event.
void onRotEncLPress(EncoderButton& eb) {
  Serial.print("Encoder longPressCount: ");
  Serial.println(eb.longPressCount());
  freqMode = !freqMode;           // Toggle the mode.
  encChange = true;
}
// Handler for the Rotary Encoder 'encoder' event.
void onRotEncEncoder(EncoderButton& eb) {
  Serial.print("Encoder incremented by: ");
  Serial.println(eb.increment());
  Serial.print("Encoder position is: ");
  Serial.println(eb.position());
  // Frequency Mode = true - the encoder changes the frequency.
  if (freqMode) {
    frequency += (eb.increment() * freqStep);
    if (frequency < FREQMIN) {
      frequency = FREQMIN;
    }
    // Sine, square and triangle waves have different upper limits.
    if (waveType == AD9833_SINE) {
      if (frequency > FREQMAXSIN) {
        frequency = FREQMAXSIN;
      }
    }
    else if (waveType == AD9833_SQUARE1) {
      if (frequency > FREQMAXSQR) {
        frequency = FREQMAXSQR;
      }
    }
    else {
      if (frequency > FREQMAXTRI) {
        frequency = FREQMAXTRI;
      }
    }
  }
  // Frequency Mode = false - the encoder changes the gain.
  else {
    rdValue += eb.increment();    // As this is an 8-bit value, it will naturally and usefully wrap, 0-255.
  }
  encChange = true;
}

// EOF