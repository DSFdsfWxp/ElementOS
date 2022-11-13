#ifndef ElementOS_statusLock
#define ElementOS_statusLock

#include "mutexLock.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class statusLock{
    public:
        statusLock(){
            this->status = 0;
        }
        void begin(){
            this->lock.take();
            this->status++;
            this->lock.give();
        }
        void end(){
            this->lock.take();
            if (this->status==0){
                this->lock.give();
                return;
            }
            this->status--;
            this->lock.give();
        }
        void take(){
            while (true){
                this->lock.take();
                if (this->status==0){
                    return;
                }
                this->lock.give();
                vTaskDelay(10);
            }
        }
        void give(){
            if (this->lock.isLock()){
                if (this->status==0){
                    this->lock.give();
                }
            }
        }
    private:
        mutexLock lock;
        int status;
};

#endif