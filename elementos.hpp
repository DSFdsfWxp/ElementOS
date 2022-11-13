#ifndef ElementOS_H
#define ElementOS_H

#include "Arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "conf/elementos-namecase.hpp"
#include "elementos-std.hpp"
#include "conf/elementos-conf.hpp"
#include "elementos-core.hpp"
#include "elementos-api.hpp"



namespace ElementOS{
    elementosApi *Api;

    class elementos{
        public:
            elementos(){
                Serial.printf("hello world from elementos::elementos()\n");

                this->corePointer = new ElementOS::elementosCore;

                this->apiPointer = new ElementOS::elementosApi(this->corePointer);
                ElementOS::Api = this->apiPointer;
                this->corePointer->session.createSession(ElementOS::n_foregroundApp);
                Serial.printf("hello world from elementos::elementos()\n");
                this->corePointer->session.addSessionFormCase<ElementOS::nameCase>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_packageName,ElementOS::n_NULL);
                Serial.printf("hello world from elementos::elementos()\n");
                this->corePointer->session.addSessionFormCase<ElementOS::nameCase>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_appClassName,ElementOS::n_NULL);
                this->corePointer->session.addSessionFormCase<ElementOS::nameCase>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_mixName,ElementOS::n_NULL);
                this->corePointer->session.addSessionFormCase<configSTACK_DEPTH_TYPE>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_endStackDeep,4096);
            }   

        private:
            ElementOS::elementosCore *corePointer;
            ElementOS::elementosApi *apiPointer;
    };

    elementos *OS;

}

#include "conf/elementos-applist.hpp"
    
namespace ElementOS{

    void setup(){
	    delay(2000);
	    ElementOS::OS = new ElementOS::elementos;
        
        /*
        *  device register part
        *  TO DO: register the device you want to use in this project here.
        */

        Api->installDevice<led_hal>(n_LED0,false,false,2,ledPwm,5000,0,12);
        //Api->installDevice<led_hal>(n_LED0,false,false,2,ledDigital);

        /* start the system core. */
        Serial.printf("hello world from ElementOS::setup()\n");
        Api->startApplication<System::core>(n_System,n_core,n_System_core,true);
        
        Serial.printf("goodbye world from ElementOS::setup()\n");
    }

    void loop(){
        /*
        *  We do not need the loop task.
        *  So we use vTaskDelay() to decrease the Cpu cost of the loop task.
        */
        vTaskDelay(10000);
    }
}

#endif
