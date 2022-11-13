#ifndef ElementOS_applicationMgr
#define ElementOS_applicationMgr

#include "inc/lfarray.hpp"
#include "inc/statusLock.hpp"
#include <stdarg.h>
#include "conf/elementos-namecase.hpp"
#include "elementos-std.hpp"
#include "errList.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"

namespace ElementOSAPI{
    class va_list_Sender{
        public:
            va_list_Sender(va_list p){
                va_copy(this->list,p);
                va_end(p);
            }

            template <typename Targ>
            Targ getArg(){
                return va_arg(this->list,Targ);
            }

            void end(){
                va_end(this->list);
            }

        private:
            va_list list;
    };
}

typedef void (*applicationMgr_applicationRemoteCallFunctionPointer)(TaskHandle_t*,void*,ElementOSAPI::va_list_Sender*);

namespace ElementOS{

class applicationMgr{
    public:

        template <typename Tappclass>
        Tappclass* addApplication(elestd_name packageName,elestd_name appClassName){
        try{
            this->lock.take();
            Serial.printf("hello world from ElementOS::applicationMgr::addApplication<...>(...)\n");
            long pos = this->packageName.indexOf(packageName);
            if (pos==-1){
                pos = this->packageName.push(packageName) - 1;
                this->appClass.pushNew();
                this->appClassName.pushNew();
                this->appRemoteCall.pushNew();
                this->appRemoteCallName.pushNew();
            }
            if (this->appClassName[pos].includes(appClassName)){
                this->lock.give();
                throw ERR_applicationMgr_appClassIsAlreadyAdded;
            }
            Serial.printf("hello world from ElementOS::applicationMgr::addApplication<...>(...)\n");
            Tappclass *newAppClass = new Tappclass;
            Serial.printf("before pushing appclass in ElementOS::applicationMgr::addApplication<...>(...)\n");
            this->appClass[pos].push((void*)newAppClass);
            Serial.printf("before pushing appclassname in ElementOS::applicationMgr::addApplication<...>(...)\n");
            this->appClassName[pos].push(appClassName);
            Serial.printf("before pushing tmpE in ElementOS::applicationMgr::addApplication<...>(...)\n");
            this->appRemoteCall[pos].pushNew();
            Serial.printf("before pushing tmpF in ElementOS::applicationMgr::addApplication<...>(...)\n");
            this->appRemoteCallName[pos].pushNew();
            this->lock.give();
            Serial.printf("goodbye world from ElementOS::applicationMgr::addApplication<...>(...)\n");
            return newAppClass;
            }catch(const char* e){
            	Serial.printf("Error: %s\n",e);
            }
        }

        template <typename Tappclass>
        void removeApplication(elestd_name packageName,elestd_name appClassName){
            this->lock.take();
            long pos = this->packageName.indexOf(packageName);
            if (pos==-1){
                this->lock.give();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(appClassName);
            if (appPos==-1){
                this->lock.give();
                throw ERR_applicationMgr_appClassNotInList;
            }
            delete (Tappclass*)(this->appClass[pos][appPos]);
            this->appClass[pos].remove(appPos);
            this->appClassName[pos].remove(appPos);
            this->appRemoteCall[pos].remove(appPos);
            this->appRemoteCallName[pos].remove(appPos);
            if (this->appClassName[pos].length()==0){
                this->appClass.remove(pos);
                this->appClassName.remove(pos);
                this->appRemoteCall.remove(pos);
                this->appRemoteCallName.remove(pos);
                this->packageName.remove(pos);
            }
            this->lock.give();
        }

        void addRemoteCall(elestd_name packageName,elestd_name appClassName,elestd_name remoteCallName,applicationMgr_applicationRemoteCallFunctionPointer remoteCallFunction){
            this->lock.take();
            long pos = this->packageName.indexOf(packageName);
            if (pos==-1){
                this->lock.give();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(appClassName);
            if (appPos==-1){
                this->lock.give();
                throw ERR_applicationMgr_appClassNotInList;
            }
            if (this->appRemoteCallName[pos][appPos].includes(remoteCallName)){
                this->lock.give();
                throw ERR_applicationMgr_appRemoteCallIsAlreadyAdded;
            }
            this->appRemoteCall[pos][appPos].push(remoteCallFunction);
            this->appRemoteCallName[pos][appPos].push(remoteCallName);
            this->lock.give();
        }

        void removeRemoteCall(elestd_name packageName,elestd_name appClassName,elestd_name remoteCallName){
            this->lock.take();
            long pos = this->packageName.indexOf(packageName);
            if (pos==-1){
                this->lock.give();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(appClassName);
            if (appPos==-1){
                this->lock.give();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long remoteCallPos = this->appRemoteCallName[pos][appPos].indexOf(remoteCallName);
            if (remoteCallPos==-1){
                this->lock.give();
                throw ERR_applicationMgr_appRemoteCallNotInList;
            }
            this->appRemoteCall[pos][appPos].remove(remoteCallPos);
            this->appRemoteCallName[pos][appPos].remove(remoteCallPos);
            this->lock.give();
        }

        template <typename Tappclass>
        Tappclass* reachApplication(elestd_name packageName,elestd_name appClassName){
            this->lock.begin();
            long pos = this->packageName.indexOf(packageName);
            if (pos==-1){
                this->lock.end();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(appClassName);
            if (appPos==-1){
                this->lock.end();
                throw ERR_applicationMgr_appClassNotInList;
            }
            Tappclass *result = (Tappclass*)(this->appClass[pos][appPos]);
            this->lock.end();
            return result;
        }

        void* reachApplicationVoid(elestd_name packageName,elestd_name appClassName){
            this->lock.begin();
            long pos = this->packageName.indexOf(packageName);
            if (pos==-1){
                this->lock.end();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(appClassName);
            if (appPos==-1){
                this->lock.end();
                throw ERR_applicationMgr_appClassNotInList;
            }
            void *result = (this->appClass[pos][appPos]);
            this->lock.end();
            return result;
        }

        applicationMgr_applicationRemoteCallFunctionPointer reachRemoteCall(elestd_name packageName,elestd_name appClassName,elestd_name remoteCallName){
            this->lock.begin();
            long pos = this->packageName.indexOf(packageName);
            if (pos==-1){
                this->lock.end();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(appClassName);
            if (appPos==-1){
                this->lock.end();
                throw ERR_applicationMgr_appClassNotInList;
            }
            long remoteCallPos = this->appRemoteCallName[pos][appPos].indexOf(remoteCallName);
            if (remoteCallPos==-1){
                this->lock.end();
                throw ERR_applicationMgr_appRemoteCallNotInList;
            }
            applicationMgr_applicationRemoteCallFunctionPointer result = (this->appRemoteCall[pos][appPos][remoteCallPos]);
            this->lock.end();
            return result;
        }

    private:
        lfArray<lfArray<void*>> appClass;
        lfArray<lfArray<lfArray<applicationMgr_applicationRemoteCallFunctionPointer>>> appRemoteCall;
        lfArray<lfArray<lfArray<elestd_name>>> appRemoteCallName;
        lfArray<lfArray<elestd_name>> appClassName;
        lfArray<elestd_name> packageName;

        statusLock lock;
};

}

#endif
