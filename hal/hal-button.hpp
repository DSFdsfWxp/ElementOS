#ifndef ElementOS_hal_button
#define ElementOS_hal_button

#include <stdarg.h>
#include "Arduino.h"

enum _button_hal_buttonDownMode{
    highIsDown,
    lowIsDown
};

class button_hal{
    public:
        /* button_hal(int buttonPort,_button_hal_buttonDownMode buttonDownMode) */
        button_hal(va_list initArg){
            this->buttonPort = va_arg(initArg,int);
            this->buttonDownMode = va_arg(initArg,_button_hal_buttonDownMode);
            va_end(initArg);
            pinMode(this->buttonPort,INPUT_PULLUP);
        }

        ~button_hal(){
            pinMode(this->buttonPort,PULLDOWN);
        }

        bool isDown(){
            switch (digitalRead(this->buttonPort)){
                case HIGH:
                    return ((this->buttonDownMode==highIsDown) ? true : false);
                case LOW:
                    return ((this->buttonDownMode==lowIsDown) ? true : false);
                default:
                    return false;
            }
            return false;
        }

    private:
        int buttonPort;
        _button_hal_buttonDownMode buttonDownMode;
};

#endif