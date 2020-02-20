# ATtiny85_thermocouple
A thermocouple thermometer with ATtiny85, capable of measuring negative temperatures

This is a modification of a project by  David Johnson-Davies, presented in http://www.technoblogy.com/show?2G9S. This project describes a thermocouple thermometer, capable of measuring temperatures up to +1350°C, using just an ATtiny85 and an OLED display. The original code is included in the distribution.
 
The project was actually able to measure temperatures from ambient to +1350°C. The modification allows the system to measure temperatures lower than the ambient, inmcluding negative temperatures, expanding the measurement range from -350°C to +1350°C.

It is well known that the accuracy and linearity of thermocouples in negative temperatures is poor, but testing to temperatures up to -30°C showed pretty nice results. It was a nice opportunity to exploit in full the capabilities of the ATtiny85 ADC. 

#  Modifications:

   HARDWARE
   Thermocouple wiring:
     + (Red wire)  -> Chip pin 3, PB4
     - (Blue wire) -> Chip pin 2, PB3

   Remove the connection of pin PB3 to GND in the original schematic.   

   SOFTWARE
   - The ADC now runs on bipolar differentiall mode
   - Negative temperatures are interpolated by a separate point array
   - The display can present negative temperatures, properly aligned 


Ilias Iliopoulos - www.fryktoria.com 1 December 2019

For any issues, you can contact me at <mailto://info@fryktoria.com> 
