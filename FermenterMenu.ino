#include <DallasTemperature.h>
#include <EasyButton.h>
#include <MenuSystem.h>
#include <OneWire.h>
#include <RotaryEncoder.h>
#include <U8x8lib.h>

#include "TempSchedule.h"
#include "MenuRenderer.cpp"


#define DEBUG

#ifdef DEBUG
    #define D(x) Serial.print(x);
    #define D2(x) Serial.println(x);
#else
    #define D(x)
    #define D2(x)
#endif


#define ENC_BTN_PIN 2
#define ENC_DT_PIN 3
#define ENC_CLK_PIN 4
#define ONE_WIRE_BUS 10
#define COOLER_RELAY_PIN 13

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

Menu newTSButton("[Nytt schema]", &onNewTSPressed);
NumericMenuItem newTSTempValue("Temperatur", nullptr, 10, -5, 30, 1.0, &formatNumericMenuItemValue);
NumericMenuItem newTSDaysValue("Dagar", nullptr, 1, 1, 30, 1.0, &formatNumericMenuItemValue);
BackMenuItem newTSConfirmButton("[Lagg till]", &onNewTSConfirmed, &ms);

Menu editTSButton("[Andra schema]", &onEditTSPressed);
NumericMenuItem editTSIndex("Schema nr", &onEditTSIndexChanged, 1, 1, 3, 1.0, &formatNumericMenuItemValue); // 1-indexed for user-readability
NumericMenuItem editTSTempValue("Temperatur", nullptr, 10, -5, 30, 1.0, &formatNumericMenuItemValue);
NumericMenuItem editTSDaysValue("Dagar", nullptr, 1, 1, 30, 1.0, &formatNumericMenuItemValue);
BackMenuItem editTSConfirmButton("[Andra]", &onEditTSConfirmed, &ms);

// *************************
// Callback functions
// *************************

// Function used in NumericMenuItem.get_formatted_value(). Formats the value as an int.
const String formatNumericMenuItemValue(const float value) {
    return String(value, 0);
}

// Resets/blocks menu for creating a new TimeSchedule.
void onNewTSPressed(Menu* menu) {
    if (schedules.getSchedulesCount() < Schedules::SCHEDULES_MAX_COUNT) {
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
}

void onEncBtnHold() {
    ms.back();
    ms.display();
}

// *************************
// Other variables
// *************************

int encPos = 0;
unsigned long lastIntervalStart = 0;
const unsigned long INTERVAL_DURATION = 5000;
float lastTemp = 0;


// *************************
// Arduino functions
// *************************

void setup() {
    Serial.begin(57600);

    // Setup rotary encoder
    encoderButton.begin();
    encoderButton.onPressed(&onEncBtnPressed);
    encoderButton.onPressedFor(750, &onEncBtnHold);

    // Setup thermometer
    sensors.begin();

    // Setup oled display
    oled.begin();
    oled.setPowerSave(false);
    oled.setFont(u8x8_font_chroma48medium8_r);

    // Setup menu for new TimeSchedule.
    ms.get_root_menu().add_menu(&newTSButton);
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
}

uint32_t last = 0;

void loop() {
    if (schedules.next()) {
        ms.back();
        ms.display();
    }

    // TODO: Only do this if a schedule is present.
    if (millis() - lastIntervalStart > INTERVAL_DURATION) {
        lastIntervalStart = millis();
        // Kolla temp och starta relä
        sensors.requestTemperatures();
        Serial.print("Temperature for the device 1 (index 0) is: ");
        Serial.println(sensors.getTempCByIndex(0));
        lastTemp = sensors.getTempCByIndex(0);

        if (&(ms.get_root_menu()) == ms.get_current_menu()) { 
            char temp[5];
            char s[16];
            dtostrf(lastTemp, 5, 1, temp);
            sprintf(s, "%s -> %i", temp, schedules.getTempSchedule(0)->getTemp());
            oled.draw1x2String(1, 5, s);
        }
    }
    
    encoder.tick();
    int newPos = encoder.getPosition();

    if (newPos != encPos) {
        if (newPos > encPos) {
            ms.next(true);
        } else if (newPos < encPos) {
            ms.prev(true);
        }
        ms.display();
        encPos = newPos;
    }

    encoderButton.read();
    Serial.println(millis() - last);
    last = millis();
}
