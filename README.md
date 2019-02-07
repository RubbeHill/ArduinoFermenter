# ArduinoFermenter
[WIP] My attempt at creating an Arduino based temperature controller for beer brewing. 
If it works well my dad and I will use it to control the temperature of a fridge, giving us precise control over the
fermentation process of our beer. :)

# Libraries
- DallasTemperature 3.8.0 https://github.com/milesburton/Arduino-Temperature-Control-Library
- EasyButton 1.0.0 https://github.com/evert-arias/EasyButton **(Modified)**
- arduino-menusystem 3.0.0 https://github.com/jonblack/arduino-menusystem **(Modified)**
- OneWire 2.3.4 https://github.com/PaulStoffregen/OneWire
- PID 1.2.0 https://github.com/br3ttb/Arduino-PID-Library
- RotaryEncoder 1.1.0 https://github.com/mathertel/RotaryEncoder
- U8g2 2.24.3 **(only U8x8)** https://github.com/olikraus/u8g2

Some of the libraries has been modified to fix bugs/add functionality that was needed. 
These can be found in the lib/ folder. All modification has a comment starting with *"// FIX:"* above them,
followed by a simple explanation of the fix.
