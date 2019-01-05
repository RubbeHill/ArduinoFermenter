#include <EasyButton.h>
#include <MenuSystem.h>
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

U8X8_SSD1306_128X64_NONAME_HW_I2C oled(/* reset=*/ U8X8_PIN_NONE);
EasyButton encoderButton(ENC_BTN_PIN);
RotaryEncoder encoder(ENC_DT_PIN, ENC_CLK_PIN);
MenuRenderer renderer(&oled);
Schedules schedules;

// Function used in NumericMenuItem.get_formatted_value(). Formats the value as an int.
const String formatNumericMenuItemValue(const float value) {
    return String(value, 0);
}

// forward declarations

void onNewTSPressed(MenuComponent* menuComponent);
void on_item1_selected(MenuComponent* p_menu_component);
void on_item2_selected(MenuComponent* p_menu_component);
void on_item3_selected(MenuComponent* p_menu_component);

// Menu variables

MenuSystem ms(renderer);

Menu newTSButton("Add temperature", &onNewTSPressed);
NumericMenuItem newTempValue("Temperature", &on_item1_selected, 0, -5, 30, 1.0, &formatNumericMenuItemValue);
NumericMenuItem newDaysValue("Days", &on_item1_selected, 0, 1, 30, 1.0, &formatNumericMenuItemValue);
BackMenuItem newTSConfirmButton("[Confirm]", &onNewTSConfirmed, &ms);

//MenuItem mm_mi1("Item 1", &on_item1_selected);
//MenuItem mm_mi2("Item 2", &on_item2_selected);
//Menu mu1("Item 3");
//NumericMenuItem mu1_mi1("Item 4", &on_item3_selected, 1, 0, 4, 1.0, &format_value_fn);

int encPos = 0;
byte shedulesCount = 0;

// Menu callback function

void onNewTSPressed(MenuComponent* menuComponent) {
    newTempValue.set_value(0);
    newDaysValue.set_value(1);
}

void onNewTSConfirmed(MenuComponent* menuComponent) {
    schedules.addTempSchedule(newTempValue.get_value(), newDaysValue.get_value());
}

void on_item1_selected(MenuComponent* p_menu_component) {
    D2("Item1 Selected")
}

void on_item2_selected(MenuComponent* p_menu_component) {
    D2("Item2 Selected")
}

void on_item3_selected(MenuComponent* p_menu_component) {
    D2("Item3 Selected")
}

void onEncBtnPressed() {
    D2("Encoder button pressed")
    ms.select();
    ms.display();
}

void onEncBtnHold() {
    D2("Encoder button held")
    ms.back();
    ms.display();
}

// Standard arduino functions

void setup() {
    Serial.begin(57600);
    encoderButton.begin();
    encoderButton.onPressed(&onEncBtnPressed);
    encoderButton.onPressedFor(750, &onEncBtnHold);

    ms.get_root_menu().add_menu(&newTSButton);
    newTSButton.add_item(&newTempValue);
    newTSButton.add_item(&newDaysValue);
    newTSButton.add_item(&newTSConfirmButton);
       
    //ms.get_root_menu().add_item(&mm_mi1);
    //ms.get_root_menu().add_item(&mm_mi2);
    //ms.get_root_menu().add_menu(&mu1);
    //mu1.add_item(&mu1_mi1);

    oled.begin();
    oled.setPowerSave(false);
    oled.setFont(u8x8_font_chroma48medium8_r);
    ms.display();
}

void loop() {
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
}
