#ifndef ElementOS_applicationMgr
#define ElementOS_applicationMgr

#include "inc/array/lfarray.hpp"
#include "errList.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"

typedef void (*applicationMgr_applicationRemoteCallFunctionPointer)(const char*,void*);

class applicationMgr{
    public:
        template <typename Tappclass>
        Tappclass* addApplication(const char *packageName,const char *appClassName){
            this->tLock_lock();
            long pos = this->packageName.indexOf(*packageName);
            if (pos==-1){
                lfArray<void*> tmpA;
                lfArray<char> tmpB;
                lfArray<lfArray<applicationMgr_applicationRemoteCallFunctionPointer>> tmpC;
                lfArray<lfArray<char>> tmpD;
                pos = this->packageName.push(*packageName) - 1;
                this->appClass.add(pos,tmpA);
                this->appClassName.add(pos,tmpB);
                this->appRemoteCall.add(pos,tmpC);
                this->appRemoteCallName.add(pos,tmpD);
            }
            if (this->appClassName[pos].includes(*appClassName)){
                throw ERR_applicationMgr_appClassIsAlreadyAdded;
            }
            Tappclass *newAppClass = new Tappclass;
            lfArray<applicationMgr_applicationRemoteCallFunctionPointer> tmpE;
            lfArray<char> tmpF;
            this->appClass[pos].push((void*)newAppClass);
            this->appClassName[pos].push(*appClassName);
            this->appRemoteCall[pos].push(tmpE);
            this->appRemoteCallName[pos].push(tmpF);
            this->tLock_unlock();
            return newAppClass;
        }

        template <typename Tappclass>
        void removeApplication(const char *packageName,const char *appClassName){
            this->tLock_lock();
            long pos = this->packageName.indexOf(*packageName);
            if (pos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(*appClassName);
            if (appPos==-1){
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
                this->packageName.remove[pos];
            }
            this->tLock_unlock();
        }

        void addRemoteCall(const char *packageName,const char *appClassName,const char *remoteCallName,applicationMgr_applicationRemoteCallFunctionPointer remoteCallFunction){
            this->tLock_lock();
            long pos = this->packageName.indexOf(*packageName);
            if (pos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(*appClassName);
            if (appPos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            if (this->appRemoteCallName[pos][appPos].includes(*remoteCallName)){
                throw ERR_applicationMgr_appRemoteCallIsAlreadyAdded;
            }
            this->appRemoteCall[pos][appPos].push(remoteCallFunction);
            this->appRemoteCallName[pos][appPos].push(*remoteCallName);
            this->tLock_unlock();
        }

        void removeRemoteCall(const char *packageName,const char *appClassName,const char *remoteCallName){
            this->tLock_lock();
            long pos = this->packageName.indexOf(*packageName);
            if (pos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(*appClassName);
            if (appPos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            long remoteCallPos = this->appRemoteCallName[pos][appPos].indexOf(*remoteCallName);
            if (remoteCallPos==-1){
                throw ERR_applicationMgr_appRemoteCallNotInList;
            }
            this->appRemoteCall[pos][appPos].remove(remoteCallPos);
            this->appRemoteCallName[pos][appPos].remove(remoteCallPos);
            this->tLock_unlock();
        }

        template <typename Tappclass>
        Tappclass* reachApplication(const char *packageName,const char *appClassName){
            this->tLock_lock();
            long pos = this->packageName.indexOf(*packageName);
            if (pos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(*appClassName);
            if (appPos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            Tappclass *result = (Tappclass*)(this->appClass[pos][appPos]);
            this->tLock_unlock();
            return result;
        }

        applicationMgr_applicationRemoteCallFunctionPointer reachRemoteCall(const char *packageName,const char *appClassName,const char *remoteCallName){
            this->tLock_lock();
            long pos = this->packageName.indexOf(*packageName);
            if (pos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            long appPos = this->appClassName[pos].indexOf(*appClassName);
            if (appPos==-1){
                throw ERR_applicationMgr_appClassNotInList;
            }
            long remoteCallPos = this->appRemoteCallName[pos][appPos].indexOf(*remoteCallName);
            if (remoteCallPos==-1){
                throw ERR_applicationMgr_appRemoteCallNotInList;
            }
            applicationMgr_applicationRemoteCallFunctionPointer result = (this->appRemoteCall[pos][appPos][remoteCallPos]);
            this->tLock_unlock();
            return result;
        }

    private:
        lfArray<lfArray<void*>> appClass;
        lfArray<lfArray<lfArray<applicationMgr_applicationRemoteCallFunctionPointer>>> appRemoteCall;
        lfArray<lfArray<lfArray<char>>> appRemoteCallName;
        lfArray<lfArray<char>> appClassName;
        lfArray<char> packageName;

        void tLock_lock(){
            vTaskSuspendAll();
        }
        
        void tLock_unlock(){
            xTaskResumeAll();
        }
};

#endif