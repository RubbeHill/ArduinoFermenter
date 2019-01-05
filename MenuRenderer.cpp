#include <MenuSystem.h>
#include <U8x8lib.h>

#define DEBUG

#ifdef DEBUG
    #define D(x) Serial.print(x);
    #define D2(x) Serial.println(x);
#else
    #define D(x)
    #define D2(x)
#endif

class MenuRenderer : public MenuComponentRenderer {
private:
    U8X8_SSD1306_128X64_NONAME_HW_I2C* _display;
    byte _currentRow;

    // Hack to allow the const methods used in MenuComponentRenderer to change the value of _currentRow.
    void setRow(int value) {
        _currentRow = value;
    }
      
public:
    MenuRenderer(U8X8_SSD1306_128X64_NONAME_HW_I2C* display) 
    : _display(display) {  
    }

    void render(Menu const& menu) const {
        D2("")
        _display->clear();
        for (int i = 0; i < menu.get_num_components(); ++i) {
            setRow(i);
            MenuComponent const* cp_m_comp = menu.get_menu_component(i);
            if (cp_m_comp->is_current())
                _display->setInverseFont(true);
            cp_m_comp->render(*this);

            if (cp_m_comp->is_current())
                D("<<< ")
            D2("")
            _display->setInverseFont(false);
        }
    }

    void render_menu_item(MenuItem const& menu_item) const {
        D(menu_item.get_name())
        
        _display->draw1x2String(0, _currentRow*2, menu_item.get_name());
    }

    void render_back_menu_item(BackMenuItem const& menu_item) const {
        D(menu_item.get_name())
        
        _display->draw1x2String(0, _currentRow*2, menu_item.get_name());
    }

    void render_numeric_menu_item(NumericMenuItem const& menu_item) const {
        D(menu_item.get_name())
        D(" ")
        D(menu_item.get_formatted_value())
        
        if (menu_item.has_focus())
            _display->setInverseFont(false);
        _display->draw1x2String(0, _currentRow*2, menu_item.get_name());
        if (menu_item.has_focus())
            _display->setInverseFont(true);
        _display->draw1x2String(13, _currentRow*2, menu_item.get_formatted_value().c_str());
        _display->setInverseFont(false);
    }

    void render_menu(Menu const& menu) const {
        D(menu.get_name())
        
        _display->draw1x2String(0, _currentRow*2, menu.get_name());
    }
};
