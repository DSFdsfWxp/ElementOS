#ifndef ElementOS_hal_button
#define ElementOS_hal_button

#include <stdarg.h>
#include "../inc/lfarray.hpp"
#include "Arduino.h"
#include "math.h"
#include "../conf/elementos-conf.hpp"

enum _led_hal_ledMode{
    ledDigital,
    ledPwm
};

#if ElementOS_conf_fixLedPwm
namespace __led_hal{
    lfArray<bool> channelList; // true -> busy false -> free
    bool ischannelListInited = false;

    int findIdleChannel(){
        if (!ischannelListInited){
            for (int i=1;i<=16;i++){
                channelList.push(false);
            }
            ischannelListInited = true;
        }
        for (int i=0;i<16;i++){
            if (channelList[i]==false){
                channelList[i] = true;
                return i;
            }
        }
        return -1;
    }
}
#endif

class led_hal{
    public:
        /* led_hal(int ledPort, _led_hal_ledMode ledMode, int freq, int channel, int resolution)
           #Tip: require (int freq, int channel, int resolution) when using mode ledPwm.
        */
        led_hal(va_list initArg){
            Serial.printf("hello world from led_hal(...)\n");
            this->ledPort = va_arg(initArg,int);
            Serial.printf("hello world from led_hal(...)\n");
            this->ledMode = (_led_hal_ledMode)(va_arg(initArg,int));
            Serial.printf("ledMode: %d\n",(int)(this->ledMode));
            Serial.printf("hello world from led_hal(...)\n");
            if (this->ledMode==ledPwm){
                this->freq = va_arg(initArg,int);
                this->channel = va_arg(initArg,int);
                this->resolution = va_arg(initArg,int);
                this->dutyMax = pow(2,this->resolution) - 1;
            }else{
                #if ElementOS_conf_fixLedPwm
                int chan = __led_hal::findIdleChannel();
                if (chan!=-1){
                    notIdleChannal = false;
                    this->channel = chan;
                    this->freq = 5000;
                    this->resolution = 12;
                    this->dutyMax = pow(2,this->resolution) - 1;
                }else{
                    this->notIdleChannal = true;
                    this->freq = -1;
                    this->channel = -1;
                    this->resolution = -1;
                }
                #else
                this->freq = -1;
                this->channel = -1;
                this->resolution = -1;
                #endif
            }
            va_end(initArg);
            Serial.printf("hello world from led_hal(...)\n");
            this->isLighted = false;
            this->duty = 0;
            this->breathUp = true;
            switch (this->ledMode){
                case ledDigital:
                    #if ElementOS_conf_fixLedPwm
                    if (this->notIdleChannal){
                        pinMode(this->ledPort,OUTPUT);
                        digitalWrite(this->ledPort,HIGH);
                    }else{
                        ledcSetup(this->channel,this->freq,this->resolution);
                        ledcAttachPin(this->ledPort,this->channel);
                        ledcWrite(this->channel,0);
                    }
                    #else
                    pinMode(this->ledPort,OUTPUT);
                    digitalWrite(this->ledPort,HIGH);
                    #endif
                    break;
                case ledPwm:
                    #if ElementOS_conf_fixLedPwm
                        if (__led_hal::channelList[this->channel]==true){
                            throw "channel not idle.";
                        }
                        ledcSetup(this->channel,this->freq,this->resolution);
                    #else
                    if (ledcChangeFrequency(this->channel,this->freq,this->resolution)==0){
                        ledcSetup(this->channel,this->freq,this->resolution);
                    }
                    #endif
                    ledcAttachPin(this->ledPort,this->channel);
                    ledcWrite(this->channel,0);
                    break;
                default:
                    break;
            }
            Serial.printf("goodbye world from led_hal(...)\n");
        }

        #if ElementOS_conf_fixLedPwm
        ~led_hal(){
            throw "led_hal can not be removed after enable ElementOS_conf_fixLedPwm.";
        }
        #else
        ~led_hal(){
            switch (this->ledMode){
                case ledDigital:
                    digitalWrite(this->ledPort,LOW);
                    break;
                case ledPwm:
                    ledcDetachPin(this->ledPort);
                    pinMode(this->ledPort,OUTPUT);
                    digitalWrite(this->ledPort,LOW);
                    break;
            }
        }
        #endif

        void on(){
            if (!(this->isLighted)){
                switch (this->ledMode){
                    case ledDigital:
                        #if ElementOS_conf_fixLedPwm
                        if (this->notIdleChannal){
                            digitalWrite(this->ledPort,HIGH);
                        }else{
                            ledcWrite(this->channel,this->dutyMax);
                        }
                        #else
                        digitalWrite(this->ledPort,HIGH);
                        #endif
                        break;
                    case ledPwm:
                        ledcWrite(this->channel,this->duty);
                        break;
                }
                this->isLighted = true;
            }
        }

        void off(){
            if (this->isLighted){
                switch (this->ledMode){
                    case ledDigital:
                        #if ElementOS_conf_fixLedPwm
                        if (this->notIdleChannal){
                            digitalWrite(this->ledPort,LOW);
                        }else{
                            ledcWrite(this->channel,0);
                        }
                        #else
                        digitalWrite(this->ledPort,LOW);
                        #endif
                        break;
                    case ledPwm:
                        ledcWrite(this->channel,0);
                        break;
                }
                this->isLighted = false;
            }
        }

        void switchMode(_led_hal_ledMode ledMode,int channel = -1,int freq = -1,int resolution = -1){
            if (this->ledMode==ledMode){
                return;
            }
            switch (ledMode){
                case ledDigital:
                    #if ElementOS_conf_fixLedPwm
                    ledcWrite(this->channel,0);
                    this->duty = 0;
                    #else
                    ledcDetachPin(this->ledPort);
                    pinMode(this->ledPort,OUTPUT);
                    digitalWrite(this->ledPort,LOW);
                    this->dutyMax = -1;
                    this->freq = -1;
                    this->channel = -1;
                    this->resolution = -1;
                    #endif
                    break;
                case ledPwm:
                    #if ElementOS_conf_fixLedPwm
                    if (this->notIdleChannal){
                        throw "all led pwm channel is busy.";
                    }
                    ledcWrite(this->channel,0);
                    this->duty = 0;
                    #else
                    if (channel<0 || freq<0 || resolution<0){
                        throw -1;
                    }
                    if (ledcChangeFrequency(this->channel,this->freq,this->resolution)==0){
                        ledcSetup(this->channel,this->freq,this->resolution);
                    }
                    ledcAttachPin(this->ledPort,this->channel);
                    ledcWrite(this->channel,0);
                    this->duty = 0;
                    this->freq = freq;
                    this->channel = channel;
                    this->resolution = resolution;
                    this->dutyMax = pow(2,this->resolution) - 1;
                    #endif
                    break;
            }
            this->ledMode = ledMode;
            this->isLighted = false;
            this->breathUp = true;
            this->duty = 0;
        }

        void pwmSetConf(int channel,int freq,int resolution){
            #if ElementOS_conf_fixLedPwm
            throw "this function is not available due to ElementOS_conf_fixLedPwm.";
            #else
            if (this->ledMode!=ledPwm){
                return;
            }
            ledcWrite(this->channel,0);
            if (this->channel!=channel){
                ledcDetachPin(this->ledPort);
            }
            if (ledcChangeFrequency(channel,freq,resolution)==0){
                ledcSetup(channel,freq,resolution);
            }
            ledcWrite(channel,0);
            if (this->channel!=channel){
                ledcAttachPin(this->ledPort,channel);
            }
            this->duty = 0;
            this->freq = freq;
            this->channel = channel;
            this->resolution = resolution;
            this->dutyMax = pow(2,this->resolution) - 1;
            this->breathUp = true;
            #endif
        }

        void pwmWrite(int duty){
            if (this->ledMode!=ledPwm){
                return;
            }
            ledcWrite(this->channel,duty);
            this->duty = duty;
        }

        void pwmSlideToDutyLoop(int duty){
            if (this->ledMode!=ledPwm){
                return;
            }
            if (duty<0 || duty>this->dutyMax){
                return;
            }
            if (this->duty>duty){
                this->duty--;
            }else if (this->duty<duty){
                this->duty++;
            }
            ledcWrite(this->channel,this->duty);
        }

        void pwmBreathLoop(int dutyMin = 0,int dutyMax = -1){
            //Serial.printf("led_hal::pwmBreathLoop()\n");
            if (this->ledMode!=ledPwm || dutyMin<0 || dutyMin>this->dutyMax || (dutyMax!=-1 && dutyMin>dutyMax) || dutyMax>this->dutyMax){
                return;
            }
            if (this->breathUp && this->duty>=((dutyMax==-1) ? this->dutyMax : dutyMax)){
                this->breathUp = false;
            }else if (!(this->breathUp) && this->duty<=dutyMin){
                this->breathUp = true;
            }
            if (this->breathUp){
                this->duty++;
            }else{
                this->duty--;
            }
            //Serial.printf("led_hal::pwmBreathLoop()\n");
            ledcWrite(this->channel,this->duty);
            //Serial.printf("led_hal::pwmBreathLoop()\n");
        }

        int getPwmChannel(){
            return ((this->ledMode==ledPwm) ? this->channel : -1);
        }

        int getPwnDuty(){
            return ((this->ledMode==ledPwm) ? this->duty : -1);
        }

        int getPwnDutyMax(){
            return ((this->ledMode==ledPwm) ? this->dutyMax : -1);
        }

        int getPwnFreq(){
            return ((this->ledMode==ledPwm) ? this->freq : -1);
        }

        int getPwnResolution(){
            return ((this->ledMode==ledPwm) ? this->resolution : -1);
        }

        bool isOn(){
            return this->isLighted;
        }

    private:
        int ledPort;
        bool isLighted;
        _led_hal_ledMode ledMode;
        int freq;
        int channel;
        int resolution;
        int duty;
        int dutyMax;
        bool breathUp;
        #if ElementOS_conf_fixLedPwm
        bool notIdleChannal;
        #endif
};

#endif