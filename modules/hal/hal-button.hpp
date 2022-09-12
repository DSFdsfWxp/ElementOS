/*
    ElementOS HAL OF BUTTON
    #Ver 1.0
    $By  Wxp
*/

#ifndef ELEMENTOS_HAL_BUTTON
#define ELEMENTOS_HAL_BUTTON

#include "Arduino.h"

class Button{
    public:
        Button(int buttonPin){
            this->_buttonPin = buttonPin;
        }
        ~Button(){}
        bool isDown(){
            
        }
    private:
        int _buttonPin;
};

#endif