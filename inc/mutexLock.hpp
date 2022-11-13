#ifndef ElementOS_mutexLock
#define ElementOS_mutexLock

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class mutexLock{
    public:
        mutexLock(){
            this->handle = xSemaphoreCreateMutex();
        }
        ~mutexLock(){
            if (this->isLock()){
                this->give();
            }
        }
        void take(){
            while (xSemaphoreTake(this->handle,1000) != pdPASS){}
        }
        void give(){
            if (xSemaphoreTake(this->handle,0) != pdPASS){
                xSemaphoreGive(this->handle);
            }
        }
        bool isLock(){
            if (xSemaphoreTake(this->handle,0) == pdPASS){
                xSemaphoreGive(this->handle);
                return false;
            }
            return true;
        }
    private:
        SemaphoreHandle_t handle;
};

#endif