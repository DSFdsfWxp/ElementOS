#ifndef ElementOS_hardwareMgr
#define ElementOS_hardwareMgr

#include "inc/lfarray.hpp"
#include "errList.hpp"

#if ElementOS_conf_enableDebug
#include "Arduino.h"
#endif

class hardwareMgr{
    public:
        template <typename Tdevicehal>
        void regDevice(Tdevicehal* halPointer,char name,bool removeable){
            this->tLock_lock();
            if (this->deviceName.includes(name)){
                throw ERR_hardwareMgr_deviceExisted;
            }
            this->devicePointer.push((void*)halPointer);
            this->deviceName.push(name);
            this->deviceRemoveable.push(removeable);
            this->tLock_unlock();
        }
        
        template <typename Tdevicehal>
        void unregDevice(char name){
            this->tLock_lock();
            if (!(this->deviceName.includes(name))){
                throw ERR_hardwareMgr_deviceNotExisted;
            }
            if (!(this->deviceRemoveable[this->deviceName.indexOf(name)])){
                throw ERR_hardwareMgr_deviceCanNotBeRemoved;
            }
            long pos = this->deviceName.indexOf(name);
            delete (Tdevicehal*)(this->devicePointer[pos]);
            this->devicePointer.remove(pos);
            this->deviceName.remove(pos);
            this->deviceRemoveable.remove(pos);
            this->tLock_unlock();
        }
        
        template <typename Tdevicehal>
        Tdevicehal* reachDevice(char name){
            if (!(this->deviceName.includes(name))){
                throw ERR_hardwareMgr_deviceNotExisted;
            }
            return (Tdevicehal*)(this->devicePointer[this->deviceName.indexOf(name)]);
        }
        #if ElementOS_conf_enableDebug
        void listDevice(){
            this->tLock_lock();
            Serial.printf("# The list of registered device(s): \n\n# Name # Removeable #\n");
            for(long i=0;i<this->deviceName.length();i++){
                Serial.printf(" \"%s\" %s\n",this->deviceName[i],(this->deviceRemoveable[i] ? "true" : "false"));
            }
            Serial.printf("\n#End of the list\n\n");
            this->tLock_unlock();
        }
        #endif
    private:
        lfArray<void*> devicePointer;
        lfArray<char> deviceName;
        lfArray<bool> deviceRemoveable;
        
        void tLock_lock(){
            /*
                Put locking operation of thread safe lock here.
            */
        }
        
        void tLock_unlock(){
            /*
                Put unlocking operation of thread safe lock here.
            */
        }
};

#endif