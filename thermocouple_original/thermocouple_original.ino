/* Tiny Thermocouple Thermometer

   David Johnson-Davies - www.technoblogy.com - 5th March 2019
   ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <Wire.h>
#include <avr/sleep.h>   

// Constants
int const commands = 0x00;
int const onecommand = 0x80;
int const data = 0x40;

// OLED 128 x 32 monochrome display **********************************************

const int OLEDAddress = 0x3C;

// Initialisation sequence for SSD1306 OLED module

int const InitLen = 15;
const uint8_t Init[InitLen] PROGMEM = {
  0xA8, // Set multiplex
  0x1F, // for 32 rows
  0x8D, // Charge pump
  0x14,
  0x20, // Memory mode
  0x01, // Vertical addressing
  0xA1, // Flip horizontally
  0xC8, // Flip vertically
  0xDA, // Set comp ins
  0x02,
  0xD9, // Set pre charge
  0xF1,
  0xDB, // Set vcom detect
  0x40, // brighter
  0xAF  // Display on
};

void InitDisplay () {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(commands);
  for (uint8_t i=0; i<InitLen; i++) Wire.write(pgm_read_byte(&Init[i]));
  Wire.endTransmission();
}

// Graphics **********************************************

int Scale; // 2 for big characters

// Character set for digits etc - stored in program memory
const uint8_t CharMap[][6] PROGMEM = {
{ 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00 }, // 30
{ 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00 }, 
{ 0x72, 0x49, 0x49, 0x49, 0x46, 0x00 }, 
{ 0x21, 0x41, 0x49, 0x4D, 0x33, 0x00 }, 
{ 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00 }, 
{ 0x27, 0x45, 0x45, 0x45, 0x39, 0x00 }, 
{ 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00 }, 
{ 0x41, 0x21, 0x11, 0x09, 0x07, 0x00 }, 
{ 0x36, 0x49, 0x49, 0x49, 0x36, 0x00 }, 
{ 0x46, 0x49, 0x49, 0x29, 0x1E, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Space
{ 0x00, 0x06, 0x09, 0x06, 0x00, 0x00 }, // degree symbol = '`'
{ 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00 }, // C
{ 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 }, // -
};

const int Space = 10;
const int Degree = 11;
const int Centigrade = 12;
const int Minus = 12;

void ClearDisplay () {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(commands);
  // Set column address range
  Wire.write(0x21); Wire.write(0); Wire.write(127);
  // Set page address range
  Wire.write(0x22); Wire.write(0); Wire.write(3);
  Wire.endTransmission();
  // Write the data in 16 32-byte transmissions
  for (int i = 0 ; i < 32; i++) {
    Wire.beginTransmission(OLEDAddress);
    Wire.write(data);
    for (int i = 0 ; i < 32; i++) Wire.write(0);
    Wire.endTransmission();
  }
}

// Converts bit pattern abcdefgh into aabbccddeeffgghh
int Stretch (int x) {
  x = (x & 0xF0)<<4 | (x & 0x0F);
  x = (x<<2 | x) & 0x3333;
  x = (x<<1 | x) & 0x5555;
  return x | x<<1;
}

// Plots a character; line = 0 to 2; column = 0 to 21
void PlotChar(int c, int line, int column) {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(commands);
  // Set column address range
  Wire.write(0x21); Wire.write(column*6); Wire.write(column*6 + Scale*6 - 1);
  // Set page address range
  Wire.write(0x22); Wire.write(line); Wire.write(line + Scale - 1);
  Wire.endTransmission();
  Wire.beginTransmission(OLEDAddress);
  Wire.write(data);
  for (uint8_t col = 0 ; col < 6; col++) {
    int bits = pgm_read_byte(&CharMap[c][col]);
    if (Scale == 1) Wire.write(bits);
    else {
      bits = Stretch(bits);
      for (int i=2; i--;) { Wire.write(bits); Wire.write(bits>>8); }
    }
  }
  Wire.endTransmission();
}

uint8_t DigitChar (unsigned int number, unsigned int divisor) {
  return (number/divisor) % 10;
}

// Display a number from -999�C to 999�C starting at line, column
void PlotTemperature (int temp, int line, int column) {
  boolean dig = false;
  unsigned int j = 1000;
  if (temp < 0) {
    PlotChar(Minus, line, column);
    temp = - temp;
    column = column + Scale;
  }
  for (int d=j; d>0; d=d/10) {
    char c = DigitChar(temp, d);
    if (c == 0 && !dig && d>1) c = Space; else dig = true;
    PlotChar(c, line, column);
    column = column + Scale;
  }
  PlotChar(Degree, line, column); column = column + Scale;
  PlotChar(Centigrade, line, column);
}

// Readings **********************************************

void SetupInternal () {
  ADMUX = 0<<REFS2 | 2<<REFS0 | 15<<MUX0;  // Temperature and 1.1V reference
  ADCSRA = 1<<ADEN | 1<<ADIE | 4<<ADPS0;   // Enable ADC, interrupt, 62.5kHz clock
  ADCSRB = 0<<ADTS0;                       // Free running
  set_sleep_mode(SLEEP_MODE_ADC);
}
  
unsigned int ReadInternal () {
  sleep_enable();
  sleep_cpu();                             // Start in ADC noise reduction mode
  return ADC;
}

void SetupThermocouple () {
  ADMUX = 0<<REFS2 | 2<<REFS0 | 7<<MUX0;   // PB4 (ADC2) +, PB3 (ADC3) was 7
  ADCSRA = 1<<ADEN | 1<<ADIE | 4<<ADPS0;   // Enable ADC, interrupt, 62.5kHz clock
  ADCSRB = 0<<ADTS0;                       // Free running 
  set_sleep_mode(SLEEP_MODE_ADC);
}
  
unsigned int ReadThermocouple () {
  sleep_enable();
  sleep_cpu();                             // Start in ADC noise reduction mode
  return ADC;
}

// Fixed points for linear interpolation
int Temperature[9] = { 0, 1684, 3370, 4995, 6613, 8269, 9993, 11805, 13751 };

// Converts a x4 ADC reading from 0 to 4095 to a temperature in degrees
int Convert (int adc) {
  int n = adc>>9;
  unsigned int difference = Temperature[n+1] - Temperature[n];
  unsigned int proportion = adc - (n<<9);
  unsigned int extra = ((unsigned long)proportion * difference)>>9;
  return (Temperature[n] + extra + 5)/10;
}
 
ISR(ADC_vect) {
// ADC conversion complete
}

// Setup **********************************************

// Calibrations
const int ADCOffset = -7;
const int InternalOffset = +5;

void setup() {
  Wire.begin();
  InitDisplay();
  ClearDisplay();
}
  
void loop () {
  //
  // Internal
  SetupInternal();
  ReadInternal();                          // Throw away first reading
  int internal = 0;
  for (int i=0; i<16; i++) internal = internal + ReadInternal();
  internal = (internal>>4) - 276 + InternalOffset;
  Scale = 1;
  PlotTemperature(internal, 3, 6);
  //
  // Thermocouple
  SetupThermocouple();
  ReadThermocouple();                      // Throw away first reading
  int reading = 0;
  for (int i=0; i<16; i++) reading = reading + ReadThermocouple();
  reading = Convert(max((reading>>2) + ADCOffset*4, 0));
  Scale = 2;
  PlotTemperature(reading + internal, 0, 0);
  delay(1000);
}
