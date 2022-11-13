#ifndef ElementOS_Api
#define ElementOS_Api

#include <stdarg.h>
#include "inc/lfarray.hpp"
#include "conf/elementos-namecase.hpp"
#include "elementos-std.hpp"
#include "elementos-core.hpp"
#include "errList.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"

typedef long lockKeyPair;
typedef void (*appClassFunctionPointer)(void*);
typedef void (*appClassRemoteCallFunctionPointer)(TaskHandle_t*,void*,ElementOSAPI::va_list_Sender*);

namespace ElementOSAPI{
    template <typename Tdevice>
    class deviceAccesser{
        public:
            deviceAccesser() : 
            key(-2),
            devicePointer(0),
            lockHandle(0) 
            {}

            deviceAccesser(const deviceAccesser<Tdevice> &p){
                throw ERR_api_deviceAccesser_invalidOpt;
            }

            void init(elestd_name deviceName,Tdevice *devicePointer,lockKeyPair lockKey,ElementOS::lockMgr *p){
                if (this->key!=-2){
                    throw ERR_api_deviceAccesser_invalidOpt;
                }
                this->key = lockKey;
                this->devicePointer = devicePointer;
                this->deviceName = deviceName;
            }
            void give(){
                if (this->key==-2){
                    return;
                }
                if (this->key!=-1){
                    this->lockHandle->giveLock(ElementOS::n_dev,this->deviceName,this->key);
                    this->key = -2;
                }
                this->devicePointer = 0;
                this->lockHandle = 0;
            }
            Tdevice* operator->(){
                if (this->key==-2){
                    throw ERR_api_deviceAccesser_notInited;
                }
                return this->devicePointer;
            }
            ~deviceAccesser(){
                this->give();
            }
        private:
            Tdevice *devicePointer;
            elestd_name deviceName;
            lockKeyPair key;
            ElementOS::lockMgr *lockHandle;
    };

    class applicationRemoteCallInfo{
        public:
            elestd_name packageName;
            elestd_name appClassName;
            elestd_name mixName;
            elestd_name callName;

            bool operator==(const applicationRemoteCallInfo &rhs){
                if (this->packageName==rhs.packageName && this->appClassName==rhs.appClassName && this->mixName==rhs.mixName && this->callName==rhs.callName){
                    return true;
                }
                return false;
            }
    };

    void applicationCreateCall(void* p);
    void applicationSuspendCall(void* p);
    void applicationResumeCall(void* p);
    void applicationDeleteCall(void* p);
    void applicationRemoteCall(void* p);

}

namespace ElementOS{

class elementosApi{
    public:
        elementosApi(elementosCore* corePointer){
            this->corePointer = corePointer;
        }

        /* session Part */

        void createSession(elestd_name name){
            this->corePointer->session.createSession(name);
        }

        void removeSession(elestd_name name){
            this->corePointer->session.removeSession(name);
        }

        template <typename Tvalue>
        void setSessionCase(elestd_name sessionName,elestd_name caseName,Tvalue value){
            try{
                this->corePointer->session.addSessionFormCase<Tvalue>(sessionName,caseName,value);
            }catch(int e){
                if (e!=ERR_sessionMgr_sessionFormCaseExisted){
                    throw e;
                }
                this->corePointer->session.setSessionFormCase<Tvalue>(sessionName,caseName,value);
            }
        }

        template <typename Tvalue>
        Tvalue getSessionCase(elestd_name sessionName,elestd_name caseName){
            this->corePointer->session.getSessionFormCase<Tvalue>(sessionName,caseName);
        }

        template <typename Tvalue>
        void cleanSessionCase(elestd_name sessionName,elestd_name caseName){
            this->corePointer->session.removeSessionFormCase<Tvalue>(sessionName,caseName);
        }
        
        /* lock Part */

        void createLock(elestd_name groupName,elestd_name lockName){
            if (groupName==ElementOS::n_dev){
                throw ERR_api_invalidLockGroupName;
            }
            this->corePointer->session.lock.createLock(groupName,lockName);
        }

        void removeLock(elestd_name groupName,elestd_name lockName,lockKeyPair lockKey){
            if (groupName==ElementOS::n_dev){
                throw ERR_api_invalidLockGroupName;
            }
            if (!this->corePointer->session.lock.verifyLock(groupName,lockName,lockKey)){
                throw ERR_api_invalidLockKey;
            }
            this->corePointer->session.lock.removeLock(groupName,lockName);
        }

        bool verifyLock(elestd_name groupName,elestd_name lockName,lockKeyPair lockKey){
            return this->corePointer->session.lock.verifyLock(groupName,lockName,lockKey);
        }

        lockKeyPair takeLock(elestd_name groupName,elestd_name lockName){
            lockKeyPair result = -2;
            while (result<0){
                result = this->corePointer->session.lock.takeLock(groupName,lockName);
                if (result==-1){
                    vTaskDelay(50);
                }
            }
            return result;
        }

        void giveLock(elestd_name groupName,elestd_name lockName,lockKeyPair lockKey){
            this->corePointer->session.lock.giveLock(groupName,lockName,lockKey);
        }

        /* contextLock Part */

        void takeContextLock(){
            this->corePointer->session.contextLock.lock();
        }

        void giveContextLock(){
            this->corePointer->session.contextLock.unlock();
        }

        /* hardware Part */

        template <typename Tdevice>
        void accessDevice(elestd_name deviceName,ElementOSAPI::deviceAccesser<Tdevice> *deviceAccesserPointer){
            if (this->corePointer->hardware.isDeviceWithLock(deviceName)){
                deviceAccesserPointer->init(deviceName,this->corePointer->hardware.reachDevice<Tdevice>(deviceName),this->takeLock(ElementOS::n_dev,deviceName),&(this->corePointer->session.lock));
            }else{
                deviceAccesserPointer->init(deviceName,this->corePointer->hardware.reachDevice<Tdevice>(deviceName),-1,&(this->corePointer->session.lock));
            }
        }
        
        template <typename Tdevice>
        void removeDevice(elestd_name deviceName,lockKeyPair lockKey){
            if (!(this->corePointer->session.lock.verifyLock(ElementOS::n_dev,deviceName,lockKey))){
                throw ERR_api_invalidLockKey;
            }
            this->corePointer->hardware.unregDevice<Tdevice>(deviceName);
            this->corePointer->session.lock.removeLock(ElementOS::n_dev,deviceName);
        }

        template <typename Tdevice>
        void removeDevice(elestd_name deviceName){
            if (this->corePointer->hardware.isDeviceWithLock(deviceName)){
                throw ERR_api_deviceNeedLock;
            }
            this->corePointer->hardware.unregDevice<Tdevice>(deviceName);
        }

        template <typename Tdevice>
        void installDevice(elestd_name deviceName,bool removeable,bool withLock,...){
            lfArray<void*> initArgs;
            va_list ap;
            va_list copy;
            va_start(ap, withLock);
            va_copy(copy,ap);
            Tdevice *newDevice = new Tdevice(copy);
            va_end(ap);
            this->corePointer->hardware.regDevice<Tdevice>(newDevice,deviceName,removeable,withLock);
            if (withLock){
                this->corePointer->session.lock.createLock(ElementOS::n_dev,deviceName);
            }
        }

        /* task Part */

        template <typename Tappclass>
        BaseType_t createTask(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority){
            Serial.printf("ptr: %p , %p\n",appClass,pvTaskCode);
            return this->corePointer->task.reachTaskCaseList(appClass->packageName)->reachTaskCase((elestd_name )(appClass->appClassName))->createTask(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority);
        }

        template <typename Tappclass>
        BaseType_t createTaskPinnedToCore(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,
                              const BaseType_t xCoreID){
            return this->corePointer->task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((elestd_name )((*appClass).appClassName))->createTaskPinnedToCore(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority,xCoreID);
        }

        template <typename Tappclass>
        BaseType_t createTaskStatic(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer){
            return this->corePointer->task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((elestd_name )((*appClass).appClassName))->createTaskStatic(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer);
        }

        template <typename Tappclass>
        BaseType_t createTaskStaticPinnedToCore(TaskFunction_t pvTaskCode,
                              Tappclass *appClass,
                              elestd_name pcName,
                              configSTACK_DEPTH_TYPE usStackDepth,
                              void *pvParameters,
                              UBaseType_t uxPriority,StackType_t *const puxStackBuffer,
                              StaticTask_t *const pxTaskBuffer,
                              const BaseType_t xCoreID){
            return this->corePointer->task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((elestd_name )((*appClass).appClassName))->createTaskStaticPinnedToCore(pvTaskCode,pcName,usStackDepth,pvParameters,uxPriority,puxStackBuffer,pxTaskBuffer,xCoreID);
        }

        template <typename Tappclass>
        void suspendTask(Tappclass *appClass,elestd_name pcName){
            this->corePointer->task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((elestd_name )((*appClass).appClassName))->suspendTask(pcName);
        }

        template <typename Tappclass>
        void resumeTask(Tappclass *appClass,elestd_name pcName){
            this->corePointer->task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((elestd_name )((*appClass).appClassName))->resumeTask(pcName);
        }

        template <typename Tappclass>
        void deleteTask(Tappclass *appClass,elestd_name pcName){
            this->corePointer->task.reachTaskCaseList((*appClass).packageName)->reachTaskCase((elestd_name )((*appClass).appClassName))->deleteTask(pcName);
        }

        /* application Part */

        template <typename TappClass>
        void killApplication(elestd_name packageName,elestd_name appClassName,elestd_name mixName,configSTACK_DEPTH_TYPE stackDeep = 4096){
            if (packageName==ElementOS::n_NULL && appClassName==ElementOS::n_NULL && mixName==ElementOS::n_NULL){
                return;
            }
            if (packageName==this->corePointer->session.getSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_packageName) && appClassName==this->corePointer->session.getSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_appClassName) && mixName==this->corePointer->session.getSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_mixName)){
                stackDeep = this->corePointer->session.getSessionFormCase<configSTACK_DEPTH_TYPE>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_endStackDeep);
            }
            lfArray<void*> *args = new lfArray<void*>;
            (*args).push((void*)(&(TappClass::end)));
            (*args).push((void*)(this->corePointer->task.reachTaskCaseList(packageName)));
            args->push((void*)(int)appClassName);
            (*args).push((void*)(int)mixName);
            this->createLock(ElementOS::n_appRemoveCall,mixName);
            lockKeyPair key = this->takeLock(ElementOS::n_appRemoveCall,mixName);
            (*args).push((void*)key);
            (*args).push((void*)(this));
            (*args).push((this->corePointer->application.reachApplicationVoid(packageName,appClassName)));
            this->corePointer->session.setSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_packageName,ElementOS::n_NULL);
            this->corePointer->session.setSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_appClassName,ElementOS::n_NULL);
            this->corePointer->session.setSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_mixName,ElementOS::n_NULL);
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTask(&(ElementOSAPI::applicationDeleteCall),ElementOS::n_end,stackDeep,(void*)args,1);
            key = takeLock(ElementOS::n_appRemoveCall,mixName);
            this->corePointer->application.removeApplication<TappClass>(packageName,appClassName);
            this->removeLock(ElementOS::n_appRemoveCall,mixName,key);
        }

        template <typename TappClass>
        void startApplication(elestd_name packageName,elestd_name appClassName,elestd_name mixName,bool isService,configSTACK_DEPTH_TYPE stackDeepStart = 4096,configSTACK_DEPTH_TYPE stackDeepKill = 4096){
            try{
                this->corePointer->task.createTaskCaseList(packageName);
            }catch(int e){
                if (e!=ERR_taskMgr_taskCaseListExisted){
                    throw e;
                }
            }
            try{
            this->corePointer->task.reachTaskCaseList(packageName)->createTaskCase(appClassName);
            }catch(int e){
                if (e!=ERR_taskMgr_taskCaseExisted){
                    throw e;
                }
                throw ERR_api_applicationIsExisted;
            }

            try{
                TappClass *newApp = this->corePointer->application.addApplication<TappClass>(packageName,appClassName);
            }catch(int e){
                Serial.printf("Error: %d\n",e);
                throw e;
            }

            if (!isService){
                this->killApplication<TappClass>(this->corePointer->session.getSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_packageName),this->corePointer->session.getSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_appClassName),this->corePointer->session.getSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_mixName));
                this->corePointer->session.setSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_packageName,packageName);
                this->corePointer->session.setSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_appClassName,appClassName);
                this->corePointer->session.setSessionFormCase<elestd_name>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_mixName,mixName);
                this->corePointer->session.setSessionFormCase<configSTACK_DEPTH_TYPE>(ElementOS::n_foregroundApp,ElementOS::n_foregroundApp_endStackDeep,stackDeepKill);
            }
            Serial.printf("hello world from ElementOS::elementosApi::startApplication<...>(...)\n");
            
            
            Serial.printf("hello world from ElementOS::elementosApi::startApplication<...>(...)\n");
            lfArray<void*> *args;
            try {
            args = new lfArray<void*>;
            
            (*args).push((void*)(&(TappClass::start)));
           
            
            (*args).push((void*)(this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)));
            (*args).push((this->corePointer->application.reachApplicationVoid(packageName,appClassName)));
            }catch(int e){
                Serial.printf("Error: %d\n",e);
            }catch(char *e){
            Serial.printf("Error: %s\n",e);
            }
            Serial.printf("hello world from ElementOS::elementosApi::startApplication<...>(...)\n");
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTask(&(ElementOSAPI::applicationCreateCall),ElementOS::n_start,stackDeepStart,(void*)args,3);
            Serial.printf("goodbye world from ElementOS::elementosApi::startApplication<...>(...)\n");
        }

        template <typename TappClass>
        void suspendApplication(elestd_name packageName,elestd_name appClassName,configSTACK_DEPTH_TYPE stackDeep = 4096){
            lfArray<void*> *args;
            (*args).push((void*)(&(TappClass::suspend)));
            (*args).push((void*)(this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)));
            (*args).push((this->corePointer->application.reachApplicationVoid(packageName,appClassName)));
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTask(&(ElementOSAPI::applicationSuspendCall),ElementOS::n_suspend,stackDeep,(void*)args,3);
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->suspendAllTask();
        }

        void suspendApplicationTask(elestd_name packageName,elestd_name appClassName,elestd_name pcName){
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->suspendAllTask(pcName);
        }

        void refreshSuspendedApplicationTaskStatus(elestd_name packageName,elestd_name appClassName){
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->refreshTask();
        }

        template <typename TappClass>
        void resumeApplication(elestd_name packageName,elestd_name appClassName,configSTACK_DEPTH_TYPE stackDeep = 4096){
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->unsuspendAllTask();
            lfArray<void*> *args;
            (*args).push((void*)(&(TappClass::suspend)));
            (*args).push((void*)(this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)));
            (*args).push((this->corePointer->application.reachApplicationVoid(packageName,appClassName)));
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTask(&(ElementOSAPI::applicationResumeCall),ElementOS::n_resume,stackDeep,(void*)args,3);
        }

        void resumeApplicationTask(elestd_name packageName,elestd_name appClassName){
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->resumeAllTask();
        }

        void deleteApplicationTask(elestd_name packageName,elestd_name appClassName,elestd_name pcName){
            this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->deleteAllTask(pcName);
        }

        template <typename TappClass>
        void addRemoteCallToApplication(TappClass *appClass,elestd_name remoteCallName,applicationMgr_applicationRemoteCallFunctionPointer remoteCallFunction){
            this->corePointer->application.addRemoteCall((*appClass).packageName,(*appClass).appClassName,remoteCallName,remoteCallFunction);
        }

        template <typename TappClass>
        void removeRemoteCallFromApplication(TappClass *appClass,elestd_name remoteCallName){
            this->corePointer->application.removeRemoteCall((*appClass).packageName,(*appClass).appClassName,remoteCallName);
        }

        void callApplicationRemoteCall(elestd_name packageName,elestd_name appClassName,elestd_name mixName,configSTACK_DEPTH_TYPE remoteCallStackSize,elestd_name remoteCallName,...){
            lfArray<void*> *callArgs = new lfArray<void*>;
            va_list ap;
            va_list copy;
            va_start(ap,remoteCallName);
            va_copy(copy,ap);
            va_end(ap);
            ElementOSAPI::va_list_Sender *copyArg = new ElementOSAPI::va_list_Sender(copy);
            (*callArgs).push((void*)(this->corePointer->application.reachRemoteCall(packageName,appClassName,remoteCallName)));
            (*callArgs).push((void*)((this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName))));
            (*callArgs).push((void*)(copyArg));
            TaskHandle_t *taskHandle;
            (*callArgs).push((void*)(taskHandle));
            (*callArgs).push((this->corePointer->application.reachApplicationVoid(packageName,appClassName)));
            this->corePointer->session.contextLock.lock();
            taskHandle = this->corePointer->task.reachTaskCaseList(packageName)->reachTaskCase(appClassName)->createTaskWithoutName(&(ElementOSAPI::applicationRemoteCall),remoteCallStackSize,(void*)(callArgs),0);
            this->corePointer->session.contextLock.unlock();
        }

        void callApplicationRemoteCall(ElementOSAPI::applicationRemoteCallInfo info,configSTACK_DEPTH_TYPE remoteCallStackSize,...){
            lfArray<void*> *callArgs = new lfArray<void*>;
            va_list ap;
            va_list copy;
            va_start(ap,remoteCallStackSize);
            va_copy(copy,ap);
            va_end(ap);
            ElementOSAPI::va_list_Sender *copyArg = new ElementOSAPI::va_list_Sender(copy);
            (*callArgs).push((void*)(this->corePointer->application.reachRemoteCall(info.packageName,info.appClassName,info.callName)));
            (*callArgs).push((void*)((this->corePointer->task.reachTaskCaseList(info.packageName)->reachTaskCase(info.appClassName))));
            (*callArgs).push((void*)(copyArg));
            TaskHandle_t *taskHandle;
            (*callArgs).push((void*)(taskHandle));
            (*callArgs).push((this->corePointer->application.reachApplicationVoid(info.packageName,info.appClassName)));
            this->corePointer->session.contextLock.lock();
            taskHandle = this->corePointer->task.reachTaskCaseList(info.packageName)->reachTaskCase(info.appClassName)->createTaskWithoutName(&(ElementOSAPI::applicationRemoteCall),remoteCallStackSize,(void*)(callArgs),0);
            this->corePointer->session.contextLock.unlock();
        }

    private:
        elementosCore* corePointer;
};
}

namespace ElementOSAPI{
     void applicationCreateCall(void* p){
     	Serial.printf("hello world from ElementOSAPI::applicationCreateCall(...)\n");
        lfArray<void*> args = *(lfArray<void*>*)p;
        Serial.printf("hello world from ElementOSAPI::applicationCreateCall(...)\n");
        appClassFunctionPointer enterPoint = (appClassFunctionPointer)(args[0]);
        ElementOS::xtaskMgr *taskHandle = (ElementOS::xtaskMgr*)(args[1]);
        enterPoint(args[2]);
        Serial.printf("hello world from ElementOSAPI::applicationCreateCall(...)\n");
        delete (lfArray<void*>*)p;
        Serial.printf("hello world from ElementOSAPI::applicationCreateCall(...)\n");
        taskHandle->deleteTask(ElementOS::n_start);
        Serial.printf("goodbye world from ElementOSAPI::applicationCreateCall(...)\n");
    }

    void applicationSuspendCall(void* p){
        lfArray<void*> args = *(lfArray<void*>*)p;
        appClassFunctionPointer enterPoint = (appClassFunctionPointer)(args[0]);
        ElementOS::xtaskMgr *taskHandle = (ElementOS::xtaskMgr*)(args[1]);
        enterPoint(args[2]);
        delete (lfArray<void*>*)p;
        taskHandle->deleteTask(ElementOS::n_suspend);
    }

    void applicationResumeCall(void* p){
        lfArray<void*> args = *(lfArray<void*>*)p;
        appClassFunctionPointer enterPoint = (appClassFunctionPointer)(args[0]);
        ElementOS::xtaskMgr *taskHandle = (ElementOS::xtaskMgr*)(args[1]);
        enterPoint(args[2]);
        delete (lfArray<void*>*)p;
        taskHandle->deleteTask(ElementOS::n_resume);
    }

    void applicationDeleteCall(void* p){
        lfArray<void*> args = *(lfArray<void*>*)p;
        appClassFunctionPointer enterPoint = (appClassFunctionPointer)(args[0]);
        lockKeyPair key = (lockKeyPair)(args[4]);
        ElementOS::elementosApi *apiHandle = (ElementOS::elementosApi*)(args[5]);
        elestd_name mixName = (elestd_name)(int)(args[3]);
        ElementOS::xtaskCaseList *taskHandle = (ElementOS::xtaskCaseList*)(args[1]);
        elestd_name appClassName = (elestd_name)(int)(args[2]);
        enterPoint(args[6]);
        (*apiHandle).giveLock(ElementOS::n_appRemoveCall,mixName,key);
        delete (lfArray<void*>*)p;
        (*taskHandle).deleteTaskCase(appClassName);
    }


    void applicationRemoteCall(void* p){
        lfArray<void*> args = *(lfArray<void*>*)p;
        appClassRemoteCallFunctionPointer enterPoint = (appClassRemoteCallFunctionPointer)(args[0]);
        ElementOS::xtaskMgr *taskHandle = (ElementOS::xtaskMgr*)(args[1]);
        ElementOSAPI::va_list_Sender *rcArgs = (ElementOSAPI::va_list_Sender*)(args[2]);
        TaskHandle_t *xtaskHandle = (TaskHandle_t*)(args[3]);
        enterPoint(xtaskHandle,args[4],rcArgs);
        delete rcArgs;
        delete (lfArray<void*>*)p;
        (*taskHandle).deleteTask(xtaskHandle);
    }

}

#endif
