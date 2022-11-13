#ifndef ElementOS_APP_system
#define ElementOS_APP_system

#include "../elementos.hpp"
#include "../elementos-std.hpp"
#include "../conf/elementos-namecase.hpp"
#include "../hal/hal-led.hpp"
#include <Arduino.h>

elestd_package System{

    elestd_application core : public elestd_baseApplication{
        public:

            ElementOSAPI::deviceAccesser<led_hal> ledHandle;
            
            core():
            elestd_baseApplication(ElementOS::n_System,ElementOS::n_core,ElementOS::n_System_core)
            {
            	Serial.printf("Hello World form System::core::core() !\n");
            }
            
            elestd_export start(void *obj){
            	Serial.printf("Hello World form System.core ! ptr: %p\n",obj);
                System::core *thisobj = (System::core*)obj;
                ElementOS::Api->accessDevice<led_hal>(ElementOS::n_LED0,&(thisobj->ledHandle));
                
                /*
                *  System entrance here.
                *  TO DO: put the code you want to run when the OS startup.
                *         (e.g.:start another app ...)
                */
               
               Serial.printf("Hello World form System.core !\n");

                ElementOS::Api->createTask<System::core>(&(System::core::task1),(System::core*)obj,ElementOS::n_task1,2048,obj,2);
                

                Serial.printf("Goodbye World form System.core !\n");

            }

            elestd_export end(void *obj){
                System::core *thisobj = (System::core*)obj;
                thisobj->ledHandle.give();
            }
            

            elestd_export task1(void* obj){
                Serial.printf("hello World form System.core.task1 ! ptr: %p\n",obj);
                System::core *thisobj = (System::core*)obj;
                Serial.printf("hello World form System.core.task1 !\n");
                for(;;){
                    thisobj->ledHandle->pwmBreathLoop();
                    vTaskDelay(1);
                }
            }

            
    };
}

#endif
