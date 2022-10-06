#ifndef ElementOS_hardwareMgr
#define ElementOS_hardwareMgr

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "inc/array/lfarray.hpp"
#include "errList.hpp"
#include "elementos-conf.hpp"

#if ElementOS_conf_enableDebug
#include "Arduino.h"
#endif

class hardwareMgr{
    public:
        template <typename Tdevicehal>
        void regDevice(Tdevicehal* halPointer,const char *name,bool removeable,bool withLock){
            this->tLock_lock();
            if (this->deviceName.includes(*name)){
                throw ERR_hardwareMgr_deviceExisted;
            }
            this->devicePointer.push((void*)halPointer);
            this->deviceName.push(*name);
            this->deviceRemoveable.push(removeable);
            this->deviceWithLock.push(withLock);
            this->tLock_unlock();
        }
        
        template <typename Tdevicehal>
        void unregDevice(const char *name){
            this->tLock_lock();
            if (!(this->deviceName.includes(*name))){
                throw ERR_hardwareMgr_deviceNotExisted;
            }
            if (!(this->deviceRemoveable[this->deviceName.indexOf(*name)])){
                throw ERR_hardwareMgr_deviceCanNotBeRemoved;
            }
            long pos = this->deviceName.indexOf(*name);
            delete (Tdevicehal*)(this->devicePointer[pos]);
            this->devicePointer.remove(pos);
            this->deviceName.remove(pos);
            this->deviceRemoveable.remove(pos);
            this->tLock_unlock();
        }
        
        template <typename Tdevicehal>
        Tdevicehal* reachDevice(const char *name){
            if (!(this->deviceName.includes(*name))){
                throw ERR_hardwareMgr_deviceNotExisted;
            }
            return (Tdevicehal*)(this->devicePointer[this->deviceName.indexOf(*name)]);
        }

        bool isDeviceWithLock(const char *name){
            return this->deviceWithLock[this->deviceName.indexOf(*name)];
        }

        #if ElementOS_conf_enableDebug
        void listDevice(){
            this->tLock_lock();
            Serial.printf("# The list of registered device(s): \n\n# Name # Removeable # WithLock #\n");
            for(long i=0;i<this->deviceName.length();i++){
                Serial.printf(" \"%s\" %s %s\n",this->deviceName[i],(this->deviceRemoveable[i] ? "true" : "false"),(this->deviceWithLock[i] ? "true" : "false"));
            }
            Serial.printf("\n#End of the list\n\n");
            this->tLock_unlock();
        }
        #endif
    private:
        lfArray<void*> devicePointer;
        lfArray<char> deviceName;
        lfArray<bool> deviceRemoveable;
        lfArray<bool> deviceWithLock;
        /*SemaphoreHandle_t accessableLock;*/
        
        void tLock_lock(){
           /*xSemaphoreTake(accessableLock,);*/
           vTaskSuspendAll();
        }
        
        void tLock_unlock(){
           xTaskResumeAll();
        }
};

#endif