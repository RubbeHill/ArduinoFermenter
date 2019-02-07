#include <DallasTemperature.h>
#include <EasyButton.h>
#include <MenuSystem.h>
#include <OneWire.h>
#include <PID_v1.h>
#include <RotaryEncoder.h>
#include <U8x8lib.h>

#include "TempSchedule.h"
#include "MenuRenderer.cpp"

#define ENC_BTN_PIN 2
#define ENC_DT_PIN 3
#define ENC_CLK_PIN 4
#define ONE_WIRE_BUS 10
#define COOLER_RELAY_PIN 12
#define HEATER_RELAY_PIN 11

// *************************
// Forward declarations
// *************************

const String formatNumericMenuItemValue(const float value);
void onNewTSPressed(Menu* menu);
void onNewTSConfirmed(MenuComponent* menuComponent);
void onEditTSPressed(Menu* menu);
void onEditTSIndexChanged(MenuComponent* menuComponent);
void onEditTSConfirmed(MenuComponent* menuComponent);

// *************************
// Hardware objects
// *************************
EasyButton encoderButton(ENC_BTN_PIN);
RotaryEncoder encoder(ENC_DT_PIN, ENC_CLK_PIN);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Schedules schedules;

// *************************
// Menu and display objects
// *************************

U8X8_SSD1306_128X64_NONAME_HW_I2C oled(/* reset=*/ U8X8_PIN_NONE);
MenuRenderer renderer(&oled);
MenuSystem ms(renderer);

Menu newTSButton("[New target]", &onNewTSPressed);
NumericMenuItem newTSIndex("Target #", nullptr, 1, 1, 3, 1.0, &formatNumericMenuItemValue); // 1-indexed for user-readability
NumericMenuItem newTSTempValue("Temperature", nullptr, 10, -5, 30, 1.0, &formatNumericMenuItemValue);
NumericMenuItem newTSDaysValue("Days", nullptr, 1, 1, 30, 1.0, &formatNumericMenuItemValue);
BackMenuItem newTSConfirmButton("[Add]", &onNewTSConfirmed, &ms);

Menu editTSButton("[Edit targets]", &onEditTSPressed);
NumericMenuItem editTSIndex("Target #", &onEditTSIndexChanged, 1, 1, 3, 1.0, &formatNumericMenuItemValue); // 1-indexed for user-readability
NumericMenuItem editTSTempValue("Temperature", nullptr, 10, -5, 30, 1.0, &formatNumericMenuItemValue);
NumericMenuItem editTSDaysValue("Days", nullptr, 1, 1, 30, 1.0, &formatNumericMenuItemValue);
BackMenuItem editTSConfirmButton("[Edit]", &onEditTSConfirmed, &ms);

// *************************
// Callback functions
// *************************

// Function used in NumericMenuItem.get_formatted_value(). Formats the value as an int.
const String formatNumericMenuItemValue(const float value) {
    return String(value, 0);
}

// Resets/blocks menu for creating a new TimeSchedule.
void onNewTSPressed(Menu* menu) {
    byte count = schedules.getSchedulesCount();
    if (count < Schedules::SCHEDULES_MAX_COUNT) {
        newTSIndex.set_value(count + 1);        // 1-index for readability
        newTSIndex.set_min_value(count + 1);    // 1-index for readability
        newTSIndex.set_max_value(count + 1);    // 1-index for readability
        newTSTempValue.set_value(10);
        newTSDaysValue.set_value(1);
    } else {
        menu->block_access();
    }
}

// Creates a new TimeSchedule.
void onNewTSConfirmed(MenuComponent* menuComponent) {
    schedules.addTempSchedule(newTSTempValue.get_value(), newTSDaysValue.get_value());
}

// Resets/blocks menu for editing current TimeSchedules.
// Starts with the values from the current TimeSchedule.
void onEditTSPressed(Menu* menu) {
    byte schedulesCount = schedules.getSchedulesCount();
    if (schedulesCount > 0) {
        editTSIndex.set_max_value(schedulesCount);
        editTS(0);
    } else {
        menu->block_access();
    }
}

// Reloads the menu for editing TimeSchedules with the values from the choosen TimeSchedule.
void onEditTSIndexChanged(MenuComponent* menuComponent) {
    editTS(editTSIndex.get_value() - 1); // -1 because editTSIndex is 1-indexed for user-readability
}

// Loads the temperature and duration values from a given TimeSchedule.
void editTS(byte index) {
    if (index < schedules.getSchedulesCount()) {
        editTSIndex.set_value(index + 1); // Display as 1-indexed for user-readability
        TempSchedule* ts = schedules.getTempSchedule(index);
        editTSTempValue.set_value(ts->getTemp());
        editTSDaysValue.set_value(ts->getDuration() / 10000);/// 86400000);
    } 
}

// Overwrites the old temperature and duration values with the new ones in the menu.
void onEditTSConfirmed(MenuComponent* menuComponent) {
    TempSchedule* ts = schedules.getTempSchedule(editTSIndex.get_value() - 1); // -1 because editTSIndex is 1-indexed for user-readability
    ts->setTemp(editTSTempValue.get_value());
    ts->setDuration(editTSDaysValue.get_value());
}

void onEncBtnPressed() {
    ms.select();
    ms.display();
    displayTempAndTarget();
}

void onEncBtnHold() {
    ms.back();
    ms.display();
    displayTempAndTarget();
}

// *************************
// Other variables
// *************************

const unsigned long WINDOW_SIZE = 5000; // 5 sec
const unsigned long COMPRESSOR_SAFETY_DURATION = 300000; // // 5 min
const double KP = 2, KI = 5, KD = 1;

double targetTemp, inputTemp, output;

PID pid(&inputTemp, &output, &targetTemp, KP, KI, KD, P_ON_M, DIRECT);

int encPos = 0;
unsigned long lastWindowStart = -WINDOW_SIZE;
unsigned long lastCompressorStop = -COMPRESSOR_SAFETY_DURATION;


// *************************
// Arduino functions
// *************************

bool updateWindow() {
    if (millis() - lastWindowStart > WINDOW_SIZE) {
        lastWindowStart += WINDOW_SIZE;
        return true;
    }
    return false;
}

void readTemp() {   
    sensors.requestTemperatures();
    inputTemp = sensors.getTempCByIndex(0);
    displayTempAndTarget();
}

void readEnc() {
    encoder.tick();
    int newPos = encoder.getPosition();

    if (newPos != encPos) {
        if (newPos > encPos) {
            ms.next(true);
        } else if (newPos < encPos) {
            ms.prev(true);
        }
        ms.display();
        displayTempAndTarget();
        encPos = newPos;
    }

    encoderButton.read();
}

void writeRelays() {
    if (schedules.getSchedulesCount() == 0) 
        return;
    
    pid.Compute();
   
    // Start/stop cooler
    if (inputTemp > targetTemp + 1 && millis() - lastCompressorStop > COMPRESSOR_SAFETY_DURATION) {
        digitalWrite(COOLER_RELAY_PIN, HIGH);
    } else if (inputTemp < targetTemp - 0.5) {
        if (digitalRead(COOLER_RELAY_PIN))
            lastCompressorStop = millis();
        digitalWrite(COOLER_RELAY_PIN, LOW);
    }

    // Start/stop heater
    if (inputTemp < targetTemp -1) {
        pid.SetMode(AUTOMATIC);
        
        if (output < millis() - lastWindowStart) {
            digitalWrite(HEATER_RELAY_PIN, HIGH);
        } else {
            digitalWrite(HEATER_RELAY_PIN, LOW);
        }
    } else if (inputTemp > targetTemp + 0.5) { // Maybe to tight to disable pid on 0.5 degree overshoot?
        pid.SetMode(MANUAL);
        digitalWrite(HEATER_RELAY_PIN, LOW);
    }
}

void displayTempAndTarget() {
    if (&(ms.get_root_menu()) == ms.get_current_menu()) { 
        char temp[6];
        char target[3];
        char s[16];
        dtostrf(inputTemp, 5, 1, temp);
        dtostrf(targetTemp, 2, 0, target);

        if (schedules.getTempSchedule(0)) {
            sprintf(s, "%s -> %s", temp, target);
        } else {
            sprintf(s, "%s -> ??", temp);
        }     
        
        oled.drawString(0, 4, "----------------");
        oled.drawString(0, 5, "Temp   Target");
        oled.draw1x2String(0, 6, s);
        if (digitalRead(COOLER_RELAY_PIN))
            oled.drawString(14, 6, "*");
        if (digitalRead(HEATER_RELAY_PIN))
            oled.drawString(14, 7, "~");
    }
}

void setup() {
    Serial.begin(57600);

    // Setup relays
    pinMode(COOLER_RELAY_PIN, OUTPUT);
    pinMode(HEATER_RELAY_PIN, OUTPUT);

    // Setup rotary encoder
    encoderButton.begin();
    encoderButton.onPressed(&onEncBtnPressed);
    encoderButton.onPressedFor(750, &onEncBtnHold);

    // Setup thermometer
    sensors.begin();
    readTemp();

    // Setup pid for heater
    pid.SetOutputLimits(0, WINDOW_SIZE);
    pid.SetMode(MANUAL); // Disable pid from start

    // Setup oled display
    oled.begin();
    oled.setPowerSave(false);
    oled.setFont(u8x8_font_chroma48medium8_r);

    // Setup menu for new TimeSchedule.
    ms.get_root_menu().add_menu(&newTSButton);
    newTSButton.add_item(&newTSIndex);
    newTSButton.add_item(&newTSTempValue);
    newTSButton.add_item(&newTSDaysValue);
    newTSButton.add_item(&newTSConfirmButton);

    // Setup menu for editing TimeSchedules.
    ms.get_root_menu().add_menu(&editTSButton);
    editTSButton.add_item(&editTSIndex);
    editTSButton.add_item(&editTSTempValue);
    editTSButton.add_item(&editTSDaysValue);
    editTSButton.add_item(&editTSConfirmButton);

    ms.display();
    displayTempAndTarget();
}

void loop() {
    if (updateWindow()) {
        readTemp();
    }
    
    // Reset stuff when the next schedule begins.
    if (schedules.next()) {
        targetTemp = schedules.getTempSchedule(0)->getTemp();
        digitalWrite(COOLER_RELAY_PIN, LOW);
        digitalWrite(HEATER_RELAY_PIN, LOW); // TODO: Make sure lastCompressorStop is correct
        pid.SetMode(MANUAL);
        ms.back();
        ms.display();
        displayTempAndTarget(); 
    } else if (schedules.getSchedulesCount() == 0) {
        digitalWrite(COOLER_RELAY_PIN, LOW);
        digitalWrite(HEATER_RELAY_PIN, LOW); // TODO: Make sure lastCompressorStop is correct
        pid.SetMode(MANUAL);
    }

    writeRelays();
    readEnc();
}
