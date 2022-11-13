#ifndef ElementOS_hardwareMgr
#define ElementOS_hardwareMgr

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "inc/lfarray.hpp"
#include "inc/statusLock.hpp"
#include "conf/elementos-namecase.hpp"
#include "elementos-std.hpp"
#include "errList.hpp"
#include "conf/elementos-conf.hpp"

#if ElementOS_conf_enableDebug
#include "Arduino.h"
#endif

namespace ElementOS{
class hardwareMgr{
    public:
        template <typename Tdevicehal>
        void regDevice(Tdevicehal* halPointer,elestd_name name,bool removeable,bool withLock){
            this->lock.take();
            if (this->deviceName.includes(name)){
                this->lock.give();
                throw ERR_hardwareMgr_deviceExisted;
            }
            this->devicePointer.push((void*)halPointer);
            this->deviceName.push(name);
            this->deviceRemoveable.push(removeable);
            this->deviceWithLock.push(withLock);
            this->lock.give();
        }
        
        template <typename Tdevicehal>
        void unregDevice(elestd_name name){
            this->lock.take();
            if (!(this->deviceName.includes(name))){
                this->lock.give();
                throw ERR_hardwareMgr_deviceNotExisted;
            }
            if (!(this->deviceRemoveable[this->deviceName.indexOf(name)])){
                this->lock.give();
                throw ERR_hardwareMgr_deviceCanNotBeRemoved;
            }
            long pos = this->deviceName.indexOf(name);
            delete (Tdevicehal*)(this->devicePointer[pos]);
            this->devicePointer.remove(pos);
            this->deviceName.remove(pos);
            this->deviceRemoveable.remove(pos);
            this->lock.give();
        }
        
        template <typename Tdevicehal>
        Tdevicehal* reachDevice(elestd_name name){
            this->lock.begin();
            if (!(this->deviceName.includes(name))){
                this->lock.end();
                throw ERR_hardwareMgr_deviceNotExisted;
            }
            Tdevicehal *result = (Tdevicehal*)(this->devicePointer[this->deviceName.indexOf(name)]);
            this->lock.end();
            return result;
        }

        bool isDeviceWithLock(elestd_name name){
            this->lock.begin();
            bool result = this->deviceWithLock[this->deviceName.indexOf(name)];
            this->lock.end();
            return result;
        }

        #if ElementOS_conf_enableDebug
        void listDevice(){
            this->lock.begin();
            Serial.printf("# The list of registered device(s): \n\n# Name # Removeable # WithLock #\n");
            for(long i=0;i<this->deviceName.length();i++){
                Serial.printf(" \"%d\" %s %s\n",this->deviceName[i],(this->deviceRemoveable[i] ? "true" : "false"),(this->deviceWithLock[i] ? "true" : "false"));
            }
            Serial.printf("\n#End of the list\n\n");
            this->lock.end();
        }
        #endif
    private:
        lfArray<void*> devicePointer;
        lfArray<elestd_name> deviceName;
        lfArray<bool> deviceRemoveable;
        lfArray<bool> deviceWithLock;
        statusLock lock;
};

}

#endif